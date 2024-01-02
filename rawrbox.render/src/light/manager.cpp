
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/plugins/clustered_light.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::shared_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};

	std::unique_ptr<Diligent::DynamicBuffer> LIGHTS::_buffer = nullptr;
	Diligent::IBufferView* LIGHTS::_bufferRead = nullptr;

	// Ambient --
	rawrbox::Colorf LIGHTS::_ambient = {0.01F, 0.01F, 0.01F, 1.F};

	// Fog --
	rawrbox::Colorf LIGHTS::_fog_color = {0.F, 0.F, 0.F, 0.F};
	float LIGHTS::_fog_density = -1.F;
	float LIGHTS::_fog_end = -1.F;
	rawrbox::FOG_TYPE LIGHTS::_fog_type = rawrbox::FOG_TYPE::FOG_EXP;
	// -----

	// PUBLIC ----
	bool LIGHTS::fullbright = false;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> LIGHTS::uniforms;
	// -------

	void LIGHTS::init() {
		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Light::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffDesc.Size = sizeof(rawrbox::LightConstants);

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &uniforms);
		}
		// -----------------------------------------

		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::LightDataVertex);
			BuffDesc.Name = "rawrbox::Light::Buffer";
			BuffDesc.Usage = Diligent::USAGE_SPARSE;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Size = BuffDesc.ElementByteStride * static_cast<uint64_t>(std::max<size_t>(_lights.size(), 1));
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = BuffDesc;

			_buffer = std::make_unique<Diligent::DynamicBuffer>(rawrbox::RENDERER->device(), dynamicBuff);
			_bufferRead = _buffer->GetBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}

		update();
	}

	void LIGHTS::shutdown() {
		RAWRBOX_DESTROY(uniforms);
		_lights.clear();
	}

	void LIGHTS::update() {
		if (_buffer == nullptr) throw std::runtime_error("[Rawrbox-LIGHT] Buffer not initialized! Did you call 'init' ?");
		if (!rawrbox::__LIGHT_DIRTY__ || _lights.empty()) return;

		auto context = rawrbox::RENDERER->context();
		auto device = rawrbox::RENDERER->device();

		// Update lights ---
		std::vector<rawrbox::LightDataVertex> lights = {};
		lights.reserve(_lights.size());

		for (auto& l : _lights) {
			if (!l->isOn()) continue;

			rawrbox::LightDataVertex light = {};

			light.position = l->getWorldPos();
			light.position.w = 1.F;

			light.color = l->getColor().rgb();
			light.intensity = l->getIntensity();

			light.direction = l->getDirection();
			light.direction.w = 1.F;

			light.radius = l->getRadius();
			light.type = l->getType();

			if (light.type == rawrbox::LightType::SPOT) {
				auto data = l->getData();

				light.penumbra = rawrbox::MathUtils::toRad(data.x) / 2.F;
				light.umbra = rawrbox::MathUtils::toRad(data.y) / 2.F;
			}

			lights.push_back(light);
		}

		// Update buffer ----
		uint64_t size = sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(std::max<size_t>(_lights.size(), 1)); // Always keep 1
		if (size > _buffer->GetDesc().Size) {
			_buffer->Resize(device, context, size, true);
		}

		rawrbox::RENDERER->context()->UpdateBuffer(_buffer->GetBuffer(), 0, sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(_lights.size()), lights.empty() ? nullptr : lights.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		rawrbox::__LIGHT_DIRTY__ = false;
		// -------
	}

	void LIGHTS::bindUniforms() {
		if (uniforms == nullptr) throw std::runtime_error("[Rawrbox-LIGHT] Buffer not initialized! Did you call 'init' ?");
		update(); // Update all lights if dirty

		auto renderer = rawrbox::RENDERER;
		auto camera = renderer->camera();
		Diligent::MapHelper<rawrbox::LightConstants> CBConstants(rawrbox::RENDERER->context(), uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

		CBConstants->g_LightSettings = {fullbright ? 1U : 0U, static_cast<uint32_t>(rawrbox::LIGHTS::count()), 0, 0}; // other light settings
		CBConstants->g_AmbientColor = _ambient;
		CBConstants->g_FogColor = _fog_color;
		CBConstants->g_FogSettings = {static_cast<float>(_fog_type), _fog_end, _fog_density, 0.F};

		// Setup light grid ----
		float nearZ = camera->getZNear();
		float farZ = camera->getZFar();
		auto gLightClustersNumZz = static_cast<float>(rawrbox::CLUSTERS_Z);

		CBConstants->g_LightGridParams = {
		    gLightClustersNumZz / std::log(farZ / nearZ),
		    (gLightClustersNumZz * std::log(nearZ)) / std::log(farZ / nearZ)};
		// --------------
	} // namespace rawrbox

	// UTILS ----
	void LIGHTS::setEnabled(bool fb) { fullbright = fb; }
	Diligent::IBufferView* LIGHTS::getBuffer() { return _bufferRead; }
	// ----

	// AMBIENT ----
	void LIGHTS::setAmbient(const rawrbox::Colorf& col) { _ambient = col; }
	const rawrbox::Colorf& LIGHTS::getAmbient() { return _ambient; }
	// ---------

	// FOG ----
	void LIGHTS::setFog(rawrbox::FOG_TYPE type, float end, float density, const rawrbox::Colorf& color) {
		_fog_type = type;
		_fog_color = color;
		_fog_density = density;
		_fog_end = end;
	}

	rawrbox::FOG_TYPE LIGHTS::getFogType() { return _fog_type; }
	const rawrbox::Colorf& LIGHTS::getFogColor() { return _fog_color; }
	float LIGHTS::getFogDensity() { return _fog_density; }
	float LIGHTS::getFogEnd() { return _fog_end; }
	// ---------

	// Light ----
	bool LIGHTS::removeLight(size_t indx) {
		if (indx > _lights.size()) return false;

		_lights.erase(_lights.begin() + indx);
		rawrbox::__LIGHT_DIRTY__ = true;
		return true;
	}

	bool LIGHTS::removeLight(rawrbox::LightBase* light) {
		bool removed = false;

		if (light == nullptr || _lights.empty()) return removed;
		for (size_t i = 0; i < _lights.size(); i++) {
			if (_lights[i].get() == light) {
				_lights.erase(_lights.begin() + i);
				removed = true;
				break;
			}
		}

		rawrbox::__LIGHT_DIRTY__ = true;
		return removed;
	}

	rawrbox::LightBase* LIGHTS::getLight(size_t indx) {
		if (indx < 0 || indx >= _lights.size()) return nullptr;
		return _lights[indx].get();
	}

	size_t LIGHTS::count() {
		return _lights.size();
	}
	// ---------

} // namespace rawrbox
