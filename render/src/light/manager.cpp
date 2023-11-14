
#include <rawrbox/render/light/manager.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::shared_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};

	std::unique_ptr<Diligent::DynamicBuffer> LIGHTS::_buffer = nullptr;
	Diligent::IBufferView* LIGHTS::_bufferRead = nullptr;

	// Ambient --
	rawrbox::Colorf LIGHTS::_ambient = {0.01F, 0.01F, 0.01F, 1.F};

	// Sun --
	rawrbox::Colorf LIGHTS::_sun_color = rawrbox::Colors::Transparent(); // No sun by default
	rawrbox::Vector3f LIGHTS::_sun_direction = {0, 0, 0};

	// Fog --
	rawrbox::Colorf LIGHTS::_fog_color = {0.F, 0.F, 0.F, 0.F};
	float LIGHTS::_fog_density = -1.F;
	float LIGHTS::_fog_end = -1.F;
	rawrbox::FOG_TYPE LIGHTS::_fog_type = rawrbox::FOG_TYPE::FOG_EXP;
	// -----

	// PUBLIC ----
	bool LIGHTS::fullbright = false;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> LIGHTS::uniforms;
	rawrbox::Event<> LIGHTS::onBufferResize = {};
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
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
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

			auto cl = l->getColor();
			auto pos = l->getWorldPos();
			auto dir = l->getDirection();

			light.position = rawrbox::Vector3f(pos.x, pos.y, pos.z);
			light.intensity = rawrbox::Vector3f(cl.r, cl.g, cl.b);
			light.direction = rawrbox::Vector3f(dir.x, dir.y, dir.z);
			light.radius = l->getRadius();
			light.type = l->getType();

			if (l->getType() == rawrbox::LightType::SPOT) {
				auto data = l->getData();
				light.innerCone = data.x;
				light.outerCone = data.y;
			} else {
				light.innerCone = 0.F;
				light.outerCone = 0.F;
			}

			lights.push_back(light);
		}

		// Update buffer ----
		uint64_t size = sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(std::max<size_t>(_lights.size(), 1)); // Always keep 1
		if (size > _buffer->GetDesc().Size) {
			_buffer->Resize(device, context, size, true);
			_bufferRead = _buffer->GetBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);

			onBufferResize();
		}

		rawrbox::RENDERER->context()->UpdateBuffer(_buffer->GetBuffer(), 0, sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(_lights.size()), lights.empty() ? nullptr : lights.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		rawrbox::__LIGHT_DIRTY__ = false;
		// -------
	}

	void LIGHTS::bindUniforms() {
		if (uniforms == nullptr) throw std::runtime_error("[Rawrbox-LIGHT] Buffer not initialized! Did you call 'init' ?");
		update(); // Update all lights if dirty

		Diligent::MapHelper<rawrbox::LightConstants> CBConstants(rawrbox::RENDERER->context(), uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

		CBConstants->g_LightSettings = {fullbright ? 1U : 0U, static_cast<uint32_t>(rawrbox::LIGHTS::count()), 0, 0}; // other light settings
		CBConstants->g_AmbientColor = _ambient.rgb();
		CBConstants->g_SunColor = _sun_color.rgb();
		CBConstants->g_SunDirection = _sun_direction;
		CBConstants->g_FogColor = _fog_color.rgb();
		CBConstants->g_FogSettings = {static_cast<float>(_fog_type), _fog_end, _fog_density, 0.F};
	} // namespace rawrbox

	// UTILS ----
	void LIGHTS::setEnabled(bool fb) { fullbright = fb; }
	Diligent::IBufferView* LIGHTS::getBuffer() { return _bufferRead; }
	// ----

	// SUN ---
	void LIGHTS::setSun(const rawrbox::Vector3f& dir, const rawrbox::Colorf& col) {
		_sun_direction = dir.normalized();
		_sun_color = col;
	}

	const rawrbox::Colorf& LIGHTS::getSunColor() { return _sun_color; }
	const rawrbox::Vector3f& LIGHTS::getSunDir() { return _sun_direction; }
	// -----

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
	void LIGHTS::removeLight(rawrbox::LightBase* light) {
		if (light == nullptr || _lights.empty()) return;
		for (size_t i = 0; i < _lights.size(); i++) {
			if (_lights[i].get() == light) {
				_lights.erase(_lights.begin() + i);
				return;
			}
		}

		rawrbox::__LIGHT_DIRTY__ = true;
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
