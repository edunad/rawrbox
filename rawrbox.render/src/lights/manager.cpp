
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/plugins/clustered.hpp>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::shared_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};
	rawrbox::LightConstants LIGHTS::_settings = {};

	std::unique_ptr<Diligent::DynamicBuffer> LIGHTS::_buffer;
	Diligent::IBufferView* LIGHTS::_bufferRead = nullptr;

	bool LIGHTS::_CONSTANTS_DIRTY = false;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> LIGHTS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Lights");
	// -------------

	// PUBLIC ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> LIGHTS::uniforms;
	// -------

	void LIGHTS::init() {
		_lights.reserve(16); // OFFSET

		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Light::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::LightConstants);

			Diligent::BufferData data;
			data.pData = &_settings;
			data.DataSize = BuffDesc.Size;

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, &data, &uniforms);
		}
		// -----------------------------------------

		// Create data --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::LightDataVertex);
			BuffDesc.Name = "RawrBox::Light::Buffer";
			BuffDesc.Usage = Diligent::USAGE_SPARSE;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * _lights.capacity();

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = BuffDesc;

			_buffer = std::make_unique<Diligent::DynamicBuffer>(rawrbox::RENDERER->device(), dynamicBuff);
			_bufferRead = _buffer->GetBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {_buffer->GetBuffer(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		// Update, DynamicBuffer does not support passing data?
		updateBuffer();
		// -----------
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

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::LightConstants), &_settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
	}

	void LIGHTS::updateBuffer() {
		if (_buffer == nullptr) RAWRBOX_CRITICAL("Buffer not initialized! Did you call 'init' ?");
		if (!rawrbox::__LIGHT_DIRTY__ || _lights.empty()) return;

		// Update lights ---
		std::vector<rawrbox::LightDataVertex> lights = {};
		lights.reserve(_lights.capacity());

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
		// ----

		auto* device = rawrbox::RENDERER->device();
		auto* context = rawrbox::RENDERER->context();

		// Resize buffer ----
		uint64_t size = sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(_lights.capacity());
		if (size > _buffer->GetDesc().Size) {
			_lights.reserve(_lights.capacity() + 16); // + OFFSET
			_buffer->Resize(device, context, sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(_lights.capacity()), true);
		}
		// --------

		auto* buffer = _buffer->GetBuffer();

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(buffer, 0, sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(lights.size()), lights.empty() ? nullptr : lights.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{buffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ---------

		rawrbox::__LIGHT_DIRTY__ = false;
	}

	void LIGHTS::update() {
		if (uniforms == nullptr) RAWRBOX_CRITICAL("Buffer not initialized! Did you call 'init' ?");

		updateBuffer();    // Update all lights if dirty
		updateConstants(); // Update buffer if dirty
	}

	// UTILS ----
	void LIGHTS::setEnabled(bool enabled) {
		auto fullbright = static_cast<uint32_t>(enabled);
		if (_settings.lightSettings.x == fullbright) return;

		_settings.lightSettings.x = fullbright;
		_CONSTANTS_DIRTY = true;
	}

	bool LIGHTS::isEnabled() { return _settings.lightSettings.x == 1U; }

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

	// LIGHT ----
	bool LIGHTS::remove(size_t indx) {
		if (indx > _lights.size()) return false;
		_lights.erase(_lights.begin() + indx);

		rawrbox::__LIGHT_DIRTY__ = true;
		_CONSTANTS_DIRTY = true;
		return true;
	}

	bool LIGHTS::remove(const rawrbox::LightBase& light) {
		if (_lights.empty()) return false;

		for (size_t i = 0; i < _lights.size(); i++) {
			if (_lights[i].get() == &light) {
				_lights.erase(_lights.begin() + i);

				rawrbox::__LIGHT_DIRTY__ = true;
				_CONSTANTS_DIRTY = true;
				return true;
			}
		}

		return false;
	}

	void LIGHTS::clear() {
		if (_lights.empty()) return;
		_lights.clear();

		rawrbox::__LIGHT_DIRTY__ = true;
		_CONSTANTS_DIRTY = true;
	}
	// ---------

} // namespace rawrbox
