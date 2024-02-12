
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/plugins/clustered.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::shared_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};
	rawrbox::LightConstants LIGHTS::_settings = {};

	std::unique_ptr<Diligent::DynamicBuffer> LIGHTS::_buffer = nullptr;
	Diligent::IBufferView* LIGHTS::_bufferRead = nullptr;

	bool LIGHTS::_CONSTANTS_DIRTY = false;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> LIGHTS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Lights");
	// -------------

	// PUBLIC ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> LIGHTS::uniforms;
	// -------

	void LIGHTS::init() {
		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Light::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::LightConstants);

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &uniforms);
			rawrbox::BindlessManager::barrier(*uniforms, rawrbox::BufferType::CONSTANT);
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

			rawrbox::BindlessManager::barrier(*_buffer->GetBuffer(), rawrbox::BufferType::SHADER);
		}

		update();
	}

	void LIGHTS::shutdown() {
		RAWRBOX_DESTROY(uniforms);

		_bufferRead = nullptr;
		_buffer.reset();

		_lights.clear();
	}

	void LIGHTS::updateConstants() {
		if (!_CONSTANTS_DIRTY) return;

		_CONSTANTS_DIRTY = false;
		_settings.lightSettings.y = static_cast<uint32_t>(count());

		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::LightConstants), &_settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		rawrbox::BindlessManager::barrier(*uniforms, rawrbox::BufferType::CONSTANT);
	}

	void LIGHTS::update() {
		if (_buffer == nullptr) throw _logger->error("Buffer not initialized! Did you call 'init' ?");
		if (!rawrbox::__LIGHT_DIRTY__ || _lights.empty()) return;

		auto context = rawrbox::RENDERER->context();
		auto device = rawrbox::RENDERER->device();

		// Update lights ---
		std::vector<rawrbox::LightDataVertex> lights = {};
		lights.reserve(_lights.size());

		for (auto& l : _lights) {
			if (!l->isActive()) continue;

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

		auto buffer = _buffer->GetBuffer();
		rawrbox::RENDERER->context()->UpdateBuffer(buffer, 0, sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(_lights.size()), lights.empty() ? nullptr : lights.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		rawrbox::BindlessManager::barrier(*buffer, rawrbox::BufferType::SHADER);

		rawrbox::__LIGHT_DIRTY__ = false;
		// -------
	}

	void LIGHTS::bindUniforms() {
		if (uniforms == nullptr) throw _logger->error("Buffer not initialized! Did you call 'init' ?");

		updateConstants(); // Update buffer if dirty
		update();          // Update all lights if dirty
	}

	// UTILS ----
	void LIGHTS::setEnabled(bool fb) {
		_settings.lightSettings.x = fb;
		_CONSTANTS_DIRTY = true;
	}

	rawrbox::LightBase* LIGHTS::getLight(size_t indx) {
		if (indx < 0 || indx >= _lights.size()) return nullptr;
		return _lights[indx].get();
	}

	size_t LIGHTS::count() { return _lights.size(); }
	Diligent::IBufferView* LIGHTS::getBuffer() { return _bufferRead; }
	// ----

	// AMBIENT ----
	void LIGHTS::setAmbient(const rawrbox::Colorf& col) {
		if (_settings.ambientColor == col) return;
		_settings.ambientColor = col;
		_CONSTANTS_DIRTY = true;
	}
	const rawrbox::Colorf& LIGHTS::getAmbient() { return _settings.ambientColor; }
	// ---------

	// FOG ----
	void LIGHTS::setFog(rawrbox::FOG_TYPE type, float end, float density, const rawrbox::Colorf& color) {
		_settings.fogSettings.x = static_cast<float>(type);
		_settings.fogSettings.y = density;
		_settings.fogSettings.z = end;
		_settings.fogColor = color;

		_CONSTANTS_DIRTY = true;
	}

	rawrbox::FOG_TYPE LIGHTS::getFogType() { return static_cast<rawrbox::FOG_TYPE>(_settings.fogSettings.x); }
	const rawrbox::Colorf& LIGHTS::getFogColor() { return _settings.fogColor; }
	float LIGHTS::getFogDensity() { return _settings.fogSettings.y; }
	float LIGHTS::getFogEnd() { return _settings.fogSettings.z; }
	// ---------

	// LIGHT ----
	bool LIGHTS::removeLight(size_t indx) {
		if (indx > _lights.size()) return false;
		_lights.erase(_lights.begin() + indx);

		rawrbox::__LIGHT_DIRTY__ = true;
		_CONSTANTS_DIRTY = true;
		return true;
	}

	bool LIGHTS::removeLight(rawrbox::LightBase* light) {
		if (light == nullptr || _lights.empty()) return false;

		for (size_t i = 0; i < _lights.size(); i++) {
			if (_lights[i].get() == light) {
				_lights.erase(_lights.begin() + i);

				rawrbox::__LIGHT_DIRTY__ = true;
				_CONSTANTS_DIRTY = true;
				return true;
			}
		}

		return false;
	}
	// ---------

} // namespace rawrbox
