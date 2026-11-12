#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/utils/pack.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/utils/barrier.hpp>

#include <algorithm>
#include <bit>
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

	const rawrbox::CameraBase* LIGHTS::_zbinCamera = nullptr;
	uint64_t LIGHTS::_zbinFrame = 0;
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
		LIGHTS::_lights.reserve(16); // OFFSET

		auto* device = rawrbox::RENDERER->device();

		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Light::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::LightConstants);

			auto settings = LIGHTS::getGPUConstants();

			Diligent::BufferData data;
			data.pData = &settings;
			data.DataSize = BuffDesc.Size;

			device->CreateBuffer(BuffDesc, &data, &LIGHTS::uniforms);
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

			device->CreateBuffer(BuffDesc, nullptr, &LIGHTS::_buffer);
			LIGHTS::_bufferRead = LIGHTS::_buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
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

			device->CreateBuffer(BuffDesc, &data, &LIGHTS::_zbinBuffer);
			LIGHTS::_zbinBufferRead = LIGHTS::_zbinBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
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

			device->CreateBuffer(BuffDesc, nullptr, &LIGHTS::_boundsBuffer);
			LIGHTS::_boundsBufferRead = LIGHTS::_boundsBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{LIGHTS::uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {LIGHTS::_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {LIGHTS::_zbinBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {LIGHTS::_boundsBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------
	}

	void LIGHTS::shutdown() {
		RAWRBOX_DESTROY(LIGHTS::uniforms);

		LIGHTS::_bufferRead = nullptr;
		RAWRBOX_DESTROY(LIGHTS::_buffer);

		LIGHTS::_zbinBufferRead = nullptr;
		RAWRBOX_DESTROY(LIGHTS::_zbinBuffer);

		LIGHTS::_boundsBufferRead = nullptr;
		RAWRBOX_DESTROY(LIGHTS::_boundsBuffer);

		LIGHTS::_lights.clear();
		LIGHTS::_data.clear();
		LIGHTS::_sorted.clear();
		LIGHTS::_viewBounds.clear();
		LIGHTS::_bounds.clear();
		LIGHTS::_entries.clear();
		LIGHTS::_zbins.clear();

		LIGHTS::_BINNED = true;
		LIGHTS::_BINS_DIRTY = true;

		LIGHTS::_settings.lightSettings = {};
	}

	rawrbox::LightConstants LIGHTS::getGPUConstants() {
		auto settings = LIGHTS::_settings;
		settings.ambientColor = LIGHTS::_settings.ambientColor.toLinear();

		if (!LIGHTS::_BINNED) settings.lightSettings.y = settings.lightSettings.z; // Directional only, they are first on the buffer
		return settings;
	}

	void LIGHTS::updateConstants() {
		if (!LIGHTS::_CONSTANTS_DIRTY) return;
		LIGHTS::_CONSTANTS_DIRTY = false;

		auto settings = LIGHTS::getGPUConstants();

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{LIGHTS::uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(LIGHTS::uniforms, 0, sizeof(rawrbox::LightConstants), &settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{LIGHTS::uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
	}

	void LIGHTS::updateData() {
		if (!rawrbox::__LIGHT_DIRTY__) return;

		rawrbox::__LIGHT_DIRTY__ = false;
		LIGHTS::_BINS_DIRTY = true;

		// Active lights ---
		LIGHTS::_data.clear();

		for (auto& l : LIGHTS::_lights) {
			if (!l->isActive()) continue;
			if (LIGHTS::_data.size() >= RB_RENDER_MAX_LIGHTS) {
				LIGHTS::_logger->warn("Too many active lights, only the first {} will be rendered", RB_RENDER_MAX_LIGHTS);
				break;
			}

			LIGHTS::_data.push_back(LIGHTS::getVertexData(*l));
		}
		// ----

		const auto total = static_cast<uint32_t>(LIGHTS::_data.size());
		if (LIGHTS::_settings.lightSettings.y != total) {
			LIGHTS::_settings.lightSettings.y = total;
			LIGHTS::_CONSTANTS_DIRTY = true;
		}
	}

	bool LIGHTS::updateBins(const rawrbox::CameraBase& camera) {
		const auto& view = camera.getViewMtx();
		const rawrbox::Vector2f nearFar = {rawrbox::MAIN_CAMERA->getZNear(), rawrbox::MAIN_CAMERA->getZFar()}; // Must match SCamera NearFar (shared, main camera)

		if (!LIGHTS::_BINS_DIRTY && LIGHTS::_zbinCamera == &camera && LIGHTS::_zbinFrame == camera.getMoveRevision() && LIGHTS::_zbinNearFar == nearFar) return false;
		LIGHTS::_BINS_DIRTY = false;

		LIGHTS::_zbinCamera = &camera;
		LIGHTS::_zbinFrame = camera.getMoveRevision();
		LIGHTS::_zbinNearFar = nearFar;

		// Split by type ---
		LIGHTS::_sorted.clear();
		LIGHTS::_bounds.clear();
		LIGHTS::_entries.clear();

		LIGHTS::_viewBounds.resize(LIGHTS::_data.size());

		for (size_t i = 0; i < LIGHTS::_data.size(); i++) {
			const auto& light = LIGHTS::_data[i];

			if (static_cast<rawrbox::LightType>(light.type & rawrbox::LightDataVertex::TYPE_MASK) == rawrbox::LightType::DIRECTIONAL) {
				LIGHTS::_sorted.push_back(light);
				LIGHTS::_bounds.emplace_back(0.F, 0.F, 0.F, -1.F); // Not binned
			} else {
				LIGHTS::_viewBounds[i] = rawrbox::ClusteredUtils::toViewSphere(view, LIGHTS::getBounds(light));
				LIGHTS::_entries.push_back(rawrbox::ClusteredUtils::getEntry(LIGHTS::_viewBounds[i], static_cast<uint32_t>(i)));
			}
		}

		const auto directional = static_cast<uint32_t>(LIGHTS::_sorted.size());
		if (LIGHTS::_settings.lightSettings.z != directional) {
			LIGHTS::_settings.lightSettings.z = directional;
			LIGHTS::_CONSTANTS_DIRTY = true;
		}
		// ----

		// Sort by depth ---
		rawrbox::ClusteredUtils::build(LIGHTS::_entries, directional, nearFar.x, nearFar.y, LIGHTS::_zbins);

		for (const auto& entry : LIGHTS::_entries) {
			LIGHTS::_sorted.push_back(LIGHTS::_data[entry.index]);
			LIGHTS::_bounds.push_back(LIGHTS::_viewBounds[entry.index]);
		}
		// ----

		// Upload ---
		auto* context = rawrbox::RENDERER->context();

		if (!LIGHTS::_sorted.empty()) {
			rawrbox::BarrierUtils::barrier({{LIGHTS::_buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->UpdateBuffer(LIGHTS::_buffer, 0, sizeof(rawrbox::LightDataVertex) * static_cast<uint64_t>(LIGHTS::_sorted.size()), LIGHTS::_sorted.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier({{LIGHTS::_buffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

			rawrbox::BarrierUtils::barrier({{LIGHTS::_boundsBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->UpdateBuffer(LIGHTS::_boundsBuffer, 0, sizeof(rawrbox::Vector4f) * static_cast<uint64_t>(LIGHTS::_bounds.size()), LIGHTS::_bounds.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier({{LIGHTS::_boundsBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		}

		rawrbox::BarrierUtils::barrier({{LIGHTS::_zbinBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateBuffer(LIGHTS::_zbinBuffer, 0, sizeof(uint32_t) * static_cast<uint64_t>(LIGHTS::_zbins.size()), LIGHTS::_zbins.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{LIGHTS::_zbinBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ---------

		return true;
	}

	bool LIGHTS::update(const rawrbox::CameraBase& camera, bool binned) {
		if (LIGHTS::uniforms == nullptr) RAWRBOX_CRITICAL("Buffer not initialized! Did you call 'init' ?");

		if (LIGHTS::_BINNED != binned) {
			LIGHTS::_BINNED = binned;
			LIGHTS::_CONSTANTS_DIRTY = true;
		}

		LIGHTS::updateData(); // Rebuild if dirty

		bool rebuilt = false;
		if (binned) rebuilt = LIGHTS::updateBins(camera);
		LIGHTS::updateConstants(); // Update buffer if dirty
		return rebuilt;
	}

	// UTILS ----
	void LIGHTS::setEnabled(bool enabled) {
		auto fullbright = static_cast<uint32_t>(enabled);
		if (LIGHTS::_settings.lightSettings.x == fullbright) return;

		LIGHTS::_settings.lightSettings.x = fullbright;
		LIGHTS::_CONSTANTS_DIRTY = true;
	}

	bool LIGHTS::isEnabled() { return LIGHTS::_settings.lightSettings.x == 1U; }

#ifdef _DEBUG
	void LIGHTS::setDebug(bool enabled) {
		auto debug = static_cast<uint32_t>(enabled);
		if (LIGHTS::_settings.lightSettings.w == debug) return;

		LIGHTS::_settings.lightSettings.w = debug;
		LIGHTS::_CONSTANTS_DIRTY = true;
	}

	bool LIGHTS::isDebug() { return LIGHTS::_settings.lightSettings.w == 1U; }
#endif

	rawrbox::LightBase* LIGHTS::getLight(size_t indx) {
		if (indx >= LIGHTS::_lights.size()) return nullptr;
		return LIGHTS::_lights[indx].get();
	}

	const std::vector<std::shared_ptr<rawrbox::LightBase>>& LIGHTS::getLights() { return LIGHTS::_lights; }

	size_t LIGHTS::count() { return LIGHTS::_lights.size(); }
	size_t LIGHTS::active() { return LIGHTS::_data.size(); }

	Diligent::IBufferView* LIGHTS::getBuffer() { return LIGHTS::_bufferRead; }
	Diligent::IBufferView* LIGHTS::getZBinBuffer() { return LIGHTS::_zbinBufferRead; }
	Diligent::IBufferView* LIGHTS::getBoundsBuffer() { return LIGHTS::_boundsBufferRead; }

	rawrbox::LightDataVertex LIGHTS::getVertexData(const rawrbox::LightBase& light) {
		rawrbox::LightDataVertex data = {};

		data.position = light.getWorldPos();
		data.radius = light.getRadius();

		data.direction = light.getDirection();

		// Encode shadow slot into the type
		const auto shadowSlot = light.getShadowSlot();
		data.type = static_cast<uint32_t>(light.getType()) | ((shadowSlot.has_value() ? shadowSlot.value() + 1U : 0U) << rawrbox::LightDataVertex::SHADOW_SHIFT);
		// ------

		const auto color = light.getColor();
		const float scale = std::max({color.r, color.g, color.b, 1.F});
		const auto radiance = (color / scale).toLinear().rgb() * (scale * light.getIntensity());

		data.radiance = {
		    static_cast<uint32_t>(rawrbox::PackUtils::toFP16(radiance.x)) | (static_cast<uint32_t>(rawrbox::PackUtils::toFP16(radiance.y)) << 16U),
		    static_cast<uint32_t>(rawrbox::PackUtils::toFP16(radiance.z))};

		if (light.getType() == rawrbox::LightType::SPOT) {
			const auto spot = light.getData();

			data.cosPenumbra = std::cos(rawrbox::MathUtils::toRad(spot.x) / 2.F);
			data.cosUmbra = std::cos(rawrbox::MathUtils::toRad(spot.y) / 2.F);
		} else if (light.getType() == rawrbox::LightType::RECT) {
			const auto rect = light.getData();
			
			// RE-USE UMBRA AND PENUMBRA
			data.cosUmbra = std::bit_cast<float>(static_cast<uint32_t>(rawrbox::PackUtils::toFP16(rect.x)) | (static_cast<uint32_t>(rawrbox::PackUtils::toFP16(rect.y)) << 16U));
			data.cosPenumbra = std::bit_cast<float>(static_cast<uint32_t>(rawrbox::PackUtils::toFP16(rect.z)) | (static_cast<uint32_t>(rawrbox::PackUtils::toFP16(rect.w)) << 16U));
		}

		return data;
	}

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
		if (LIGHTS::_settings.ambientColor == col) return;

		LIGHTS::_settings.ambientColor = col;
		LIGHTS::_CONSTANTS_DIRTY = true;
	}

	const rawrbox::Colorf& LIGHTS::getAmbient() { return LIGHTS::_settings.ambientColor; }
	// ---------

	// LIGHT ----
	bool LIGHTS::remove(size_t indx) {
		if (indx >= LIGHTS::_lights.size()) return false;
		LIGHTS::_lights.erase(LIGHTS::_lights.begin() + indx);

		rawrbox::__LIGHT_DIRTY__ = true;
		LIGHTS::_CONSTANTS_DIRTY = true;
		return true;
	}

	bool LIGHTS::remove(const rawrbox::LightBase& light) {
		if (LIGHTS::_lights.empty()) return false;

		for (size_t i = 0; i < LIGHTS::_lights.size(); i++) {
			if (LIGHTS::_lights[i].get() == &light) {
				LIGHTS::_lights.erase(LIGHTS::_lights.begin() + i);

				rawrbox::__LIGHT_DIRTY__ = true;
				LIGHTS::_CONSTANTS_DIRTY = true;
				return true;
			}
		}

		return false;
	}

	void LIGHTS::clear() {
		if (LIGHTS::_lights.empty()) return;
		LIGHTS::_lights.clear();

		rawrbox::__LIGHT_DIRTY__ = true;
		LIGHTS::_CONSTANTS_DIRTY = true;
	}
	// ---------

} // namespace rawrbox