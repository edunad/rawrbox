#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/utils/pack.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/utils/barrier.hpp>

#include <algorithm>
#include <cmath>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::shared_ptr<rawrbox::LightBase>> LIGHTS::_lights = {};
	rawrbox::LightConstants LIGHTS::_settings = {};

	// BINNING ---
	std::vector<rawrbox::LightDataVertex> LIGHTS::_data = {};
	std::vector<rawrbox::LightDataVertex> LIGHTS::_sorted = {};

	std::vector<rawrbox::Vector4f> LIGHTS::_viewBounds = {};
	std::vector<rawrbox::Vector4f> LIGHTS::_bounds = {};

	std::vector<rawrbox::ZBinEntry> LIGHTS::_entries = {};
	std::vector<uint32_t> LIGHTS::_zbins = {};

	rawrbox::Matrix4x4 LIGHTS::_zbinView = {};
	rawrbox::Vector2f LIGHTS::_zbinNearFar = {};
	bool LIGHTS::_BINS_DIRTY = true;
	// -----------

	// BUFFERS ---
	Diligent::RefCntAutoPtr<Diligent::IBuffer> LIGHTS::_buffer;
	Diligent::IBufferView* LIGHTS::_bufferRead = nullptr;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> LIGHTS::_zbinBuffer;
	Diligent::IBufferView* LIGHTS::_zbinBufferRead = nullptr;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> LIGHTS::_boundsBuffer;
	Diligent::IBufferView* LIGHTS::_boundsBufferRead = nullptr;
	// -----------

	bool LIGHTS::_CONSTANTS_DIRTY = false;
	bool LIGHTS::_BINNED = true;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> LIGHTS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Lights");
	// -------------

	// PUBLIC ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> LIGHTS::uniforms;
	// -------

	void LIGHTS::init() {
		_lights.reserve(16); // OFFSET

		auto* device = rawrbox::RENDERER->device();

		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Light::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::LightConstants);

			auto settings = getGPUConstants();

			Diligent::BufferData data;
			data.pData = &settings;
			data.DataSize = BuffDesc.Size;

			device->CreateBuffer(BuffDesc, &data, &uniforms);
		}
		// -----------------------------------------

		// Create data --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::LightDataVertex);
			BuffDesc.Name = "RawrBox::Light::Buffer";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_MAX_LIGHTS;

			device->CreateBuffer(BuffDesc, nullptr, &_buffer);
			_bufferRead = _buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// Create z-bins --
		{
			std::vector<uint32_t> empty(RB_RENDER_ZBINS, rawrbox::ClusteredUtils::EMPTY_BIN);

			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(uint32_t);
			BuffDesc.Name = "RawrBox::Light::ZBins";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_ZBINS;

			Diligent::BufferData data;
			data.pData = empty.data();
			data.DataSize = BuffDesc.Size;

			device->CreateBuffer(BuffDesc, &data, &_zbinBuffer);
			_zbinBufferRead = _zbinBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// Create bounds --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Vector4f);
			BuffDesc.Name = "RawrBox::Light::Bounds";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_MAX_LIGHTS;

			device->CreateBuffer(BuffDesc, nullptr, &_boundsBuffer);
			_boundsBufferRead = _boundsBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {_zbinBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {_boundsBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------
	}

	void LIGHTS::shutdown() {
		RAWRBOX_DESTROY(uniforms);

		_bufferRead = nullptr;
		RAWRBOX_DESTROY(_buffer);

		_zbinBufferRead = nullptr;
		RAWRBOX_DESTROY(_zbinBuffer);

		_boundsBufferRead = nullptr;
		RAWRBOX_DESTROY(_boundsBuffer);

		_lights.clear();
		_data.clear();
		_sorted.clear();
		_viewBounds.clear();
		_bounds.clear();
		_entries.clear();
		_zbins.clear();

		_BINNED = true;
		_BINS_DIRTY = true;

		_settings.lightSettings = {};
	}

	rawrbox::LightConstants LIGHTS::getGPUConstants() {
		auto settings = _settings;
		settings.ambientColor = _settings.ambientColor.toLinear();

		if (!_BINNED) settings.lightSettings.y = settings.lightSettings.z; // Directional only, they are first on the buffer
		return settings;
	}

	void LIGHTS::updateConstants() {
		if (!_CONSTANTS_DIRTY) return;
		_CONSTANTS_DIRTY = false;

		auto settings = getGPUConstants();

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::LightConstants), &settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
	}

	void LIGHTS::updateData() {
		if (!rawrbox::__LIGHT_DIRTY__) return;

		rawrbox::__LIGHT_DIRTY__ = false;
		_BINS_DIRTY = true;

		// Active lights ---
		_data.clear();

		for (auto& l : _lights) {
			if (!l->isActive()) continue;
			if (_data.size() >= RB_RENDER_MAX_LIGHTS) {
				_logger->warn("Too many active lights, only the first {} will be rendered", RB_RENDER_MAX_LIGHTS);
				break;
			}

			rawrbox::LightDataVertex light = {};

			light.position = l->getWorldPos();
			light.radius = l->getRadius();

			light.direction = l->getDirection();
			light.type = static_cast<uint32_t>(l->getType()); // Shadow index = 0

			const auto color = l->getColor();
			const float scale = std::max({color.r, color.g, color.b, 1.F});
			const auto radiance = (color / scale).toLinear().rgb() * (scale * l->getIntensity());

			light.radiance = {
			    static_cast<uint32_t>(rawrbox::PackUtils::toFP16(radiance.x)) | (static_cast<uint32_t>(rawrbox::PackUtils::toFP16(radiance.y)) << 16U),
			    static_cast<uint32_t>(rawrbox::PackUtils::toFP16(radiance.z))};

			if (l->getType() == rawrbox::LightType::SPOT) {
				auto data = l->getData();

				light.cosPenumbra = std::cos(rawrbox::MathUtils::toRad(data.x) / 2.F);
				light.cosUmbra = std::cos(rawrbox::MathUtils::toRad(data.y) / 2.F);
			}

			_data.push_back(light);
		}
		// ----

		const auto total = static_cast<uint32_t>(_data.size());
		if (_settings.lightSettings.y != total) {
			_settings.lightSettings.y = total;
			_CONSTANTS_DIRTY = true;
		}
	}

	bool LIGHTS::updateBins(const rawrbox::CameraBase& camera) {
		const auto& view = camera.getViewMtx();
		const rawrbox::Vector2f nearFar = {rawrbox::MAIN_CAMERA->getZNear(), rawrbox::MAIN_CAMERA->getZFar()}; // Must match SCamera NearFar (shared, main camera)

		if (!_BINS_DIRTY && _zbinView == view && _zbinNearFar == nearFar) return false;
		_BINS_DIRTY = false;

		_zbinView = view;
		_zbinNearFar = nearFar;

		// Split by type ---
		_sorted.clear();
		_bounds.clear();
		_entries.clear();

		_viewBounds.resize(_data.size());

		for (size_t i = 0; i < _data.size(); i++) {
			const auto& light = _data[i];

			if (static_cast<rawrbox::LightType>(light.type & rawrbox::LightDataVertex::TYPE_MASK) == rawrbox::LightType::DIRECTIONAL) {
				_sorted.push_back(light);
				_bounds.emplace_back(0.F, 0.F, 0.F, -1.F); // Not binned
			} else {
				_viewBounds[i] = rawrbox::ClusteredUtils::toViewSphere(view, getBounds(light));
				_entries.push_back(rawrbox::ClusteredUtils::getEntry(_viewBounds[i], static_cast<uint32_t>(i)));
			}
		}

		const auto directional = static_cast<uint32_t>(_sorted.size());
		if (_settings.lightSettings.z != directional) {
			_settings.lightSettings.z = directional;
			_CONSTANTS_DIRTY = true;
		}
		// ----

		// Sort by depth ---
		rawrbox::ClusteredUtils::build(_entries, directional, nearFar.x, nearFar.y, _zbins);

		for (const auto& entry : _entries) {
			_sorted.push_back(_data[entry.index]);
			_bounds.push_back(_viewBounds[entry.index]);
		}
		// ----

		// Upload ---
		auto* context = rawrbox::RENDERER->context();

		if (!_sorted.empty()) {
			rawrbox::BarrierUtils::barrier({{_buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->UpdateBuffer(_buffer, 0, sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(_sorted.size()), _sorted.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier({{_buffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

			rawrbox::BarrierUtils::barrier({{_boundsBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->UpdateBuffer(_boundsBuffer, 0, sizeof(rawrbox::Vector4f) * static_cast<uint64_t>(_bounds.size()), _bounds.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier({{_boundsBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		}

		rawrbox::BarrierUtils::barrier({{_zbinBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateBuffer(_zbinBuffer, 0, sizeof(uint32_t) * static_cast<uint64_t>(_zbins.size()), _zbins.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{_zbinBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ---------

		return true;
	}

	bool LIGHTS::update(const rawrbox::CameraBase& camera, bool binned) {
		if (uniforms == nullptr) RAWRBOX_CRITICAL("Buffer not initialized! Did you call 'init' ?");

		if (_BINNED != binned) {
			_BINNED = binned;
			_CONSTANTS_DIRTY = true;
		}

		updateData(); // Rebuild if dirty

		bool rebuilt = false;
		if (binned) rebuilt = updateBins(camera);
		updateConstants(); // Update buffer if dirty
		return rebuilt;
	}

	// UTILS ----
	void LIGHTS::setEnabled(bool enabled) {
		auto fullbright = static_cast<uint32_t>(enabled);
		if (_settings.lightSettings.x == fullbright) return;

		_settings.lightSettings.x = fullbright;
		_CONSTANTS_DIRTY = true;
	}

	bool LIGHTS::isEnabled() { return _settings.lightSettings.x == 1U; }

	void LIGHTS::setDebug(bool enabled) {
		auto debug = static_cast<uint32_t>(enabled);
		if (_settings.lightSettings.w == debug) return;

		_settings.lightSettings.w = debug;
		_CONSTANTS_DIRTY = true;
	}

	bool LIGHTS::isDebug() { return _settings.lightSettings.w == 1U; }

	rawrbox::LightBase* LIGHTS::getLight(size_t indx) {
		if (indx >= _lights.size()) return nullptr;
		return _lights[indx].get();
	}

	size_t LIGHTS::count() { return _lights.size(); }
	size_t LIGHTS::active() { return _data.size(); }

	Diligent::IBufferView* LIGHTS::getBuffer() { return _bufferRead; }
	Diligent::IBufferView* LIGHTS::getZBinBuffer() { return _zbinBufferRead; }
	Diligent::IBufferView* LIGHTS::getBoundsBuffer() { return _boundsBufferRead; }

	rawrbox::Vector4f LIGHTS::getBounds(const rawrbox::LightDataVertex& light) {
		const float dirLength = light.direction.length();

		if (static_cast<rawrbox::LightType>(light.type & rawrbox::LightDataVertex::TYPE_MASK) != rawrbox::LightType::SPOT || dirLength <= 0.F)
			return {light.position.x, light.position.y, light.position.z, light.radius};

		// https://bartwronski.com/2017/04/13/cull-that-cone/
		const auto dir = light.direction / dirLength;
		const float cosAngle = light.cosUmbra;
		if (cosAngle <= 0.F) return {light.position.x, light.position.y, light.position.z, light.radius}; // Cone wider than 180 degrees

		rawrbox::Vector3f center = {};
		float radius = 0.F;

		if (cosAngle < 0.70710678F) { // > 45
			center = light.position + dir * (cosAngle * light.radius);
			radius = std::sqrt(std::max(0.F, 1.F - (cosAngle * cosAngle))) * light.radius;
		} else {
			radius = light.radius / (2.F * cosAngle);
			center = light.position + dir * radius;
		}

		return {center.x, center.y, center.z, radius};
	}
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
		if (indx >= _lights.size()) return false;
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
