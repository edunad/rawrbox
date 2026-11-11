#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

#include <algorithm>

namespace rawrbox {
	// PRIVATE ----
	std::vector<rawrbox::Decal> DECALS::_decals = {};

	// BINNING ---
	std::vector<rawrbox::Decal> DECALS::_sorted = {};

	std::vector<rawrbox::Vector4f> DECALS::_viewBounds = {};
	std::vector<rawrbox::Vector4f> DECALS::_bounds = {};

	std::vector<rawrbox::ZBinEntry> DECALS::_entries = {};
	std::vector<uint32_t> DECALS::_zbins = {};

	const rawrbox::CameraBase* DECALS::_zbinCamera = nullptr;
	uint64_t DECALS::_zbinFrame = 0;
	rawrbox::Vector2f DECALS::_zbinNearFar = {};
	// -----------

	// BUFFERS ---
	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::_buffer;
	Diligent::IBufferView* DECALS::_bufferRead = nullptr;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::_zbinBuffer;
	Diligent::IBufferView* DECALS::_zbinBufferRead = nullptr;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::_boundsBuffer;
	Diligent::IBufferView* DECALS::_boundsBufferRead = nullptr;
	// -----------

	bool DECALS::_CONSTANTS_DIRTY = false;
	bool DECALS::_BINNED = true;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> DECALS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Decals");
	// -------------

	// PUBLIC ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::uniforms;
	// -------

	void DECALS::init() {
		DECALS::_decals.reserve(16); // OFFSET

		auto* device = rawrbox::RENDERER->device();

		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Decals::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::Vector4u);

			device->CreateBuffer(BuffDesc, nullptr, &DECALS::uniforms);
		}
		// -----------------------------------------

		// Create data --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Decal);
			BuffDesc.Name = "RawrBox::Decals::Buffer";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_MAX_DECALS;

			device->CreateBuffer(BuffDesc, nullptr, &DECALS::_buffer);
			DECALS::_bufferRead = DECALS::_buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// -----------

		// Zbins --
		{
			std::vector<uint32_t> empty(RB_RENDER_ZBINS, rawrbox::ClusteredUtils::EMPTY_BIN);

			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(uint32_t);
			BuffDesc.Name = "RawrBox::Decals::ZBins";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_ZBINS;

			Diligent::BufferData data;
			data.pData = empty.data();
			data.DataSize = BuffDesc.Size;

			device->CreateBuffer(BuffDesc, &data, &DECALS::_zbinBuffer);
			DECALS::_zbinBufferRead = DECALS::_zbinBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// Create bounds --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Vector4f);
			BuffDesc.Name = "RawrBox::Decals::Bounds";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_MAX_DECALS;

			device->CreateBuffer(BuffDesc, nullptr, &DECALS::_boundsBuffer);
			DECALS::_boundsBufferRead = DECALS::_boundsBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{DECALS::uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {DECALS::_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {DECALS::_zbinBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {DECALS::_boundsBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		DECALS::_CONSTANTS_DIRTY = true;
	}

	void DECALS::shutdown() {
		RAWRBOX_DESTROY(DECALS::uniforms);

		DECALS::_bufferRead = nullptr;
		RAWRBOX_DESTROY(DECALS::_buffer);

		DECALS::_zbinBufferRead = nullptr;
		RAWRBOX_DESTROY(DECALS::_zbinBuffer);

		DECALS::_boundsBufferRead = nullptr;
		RAWRBOX_DESTROY(DECALS::_boundsBuffer);

		DECALS::_decals.clear();
		DECALS::_sorted.clear();
		DECALS::_viewBounds.clear();
		DECALS::_bounds.clear();
		DECALS::_entries.clear();
		DECALS::_zbins.clear();
	}

	void DECALS::updateConstants() {
		if (!DECALS::_CONSTANTS_DIRTY) return;
		DECALS::_CONSTANTS_DIRTY = false;

		const auto total = DECALS::_BINNED ? static_cast<uint32_t>(std::min<size_t>(DECALS::_decals.size(), RB_RENDER_MAX_DECALS)) : 0U;
		rawrbox::Vector4u settings = {total, 0, 0, 0};

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{DECALS::uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(DECALS::uniforms, 0, sizeof(rawrbox::Vector4u), &settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{DECALS::uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
	}

	bool DECALS::updateBins(const rawrbox::CameraBase& camera) {
		const auto& view = camera.getViewMtx();
		const rawrbox::Vector2f nearFar = {rawrbox::MAIN_CAMERA->getZNear(), rawrbox::MAIN_CAMERA->getZFar()}; // Must match SCamera NearFar (shared, main camera)

		if (!rawrbox::__DECALS_DIRTY__ && DECALS::_zbinCamera == &camera && DECALS::_zbinFrame == camera.getMoveRevision() && DECALS::_zbinNearFar == nearFar) return false;
		rawrbox::__DECALS_DIRTY__ = false;

		DECALS::_zbinCamera = &camera;
		DECALS::_zbinFrame = camera.getMoveRevision();
		DECALS::_zbinNearFar = nearFar;

		// Sort by depth + bin ---
		DECALS::_entries.clear();
		DECALS::_viewBounds.resize(std::min<size_t>(DECALS::_decals.size(), RB_RENDER_MAX_DECALS));

		for (size_t i = 0; i < DECALS::_decals.size(); i++) {
			if (i >= RB_RENDER_MAX_DECALS) {
				DECALS::_logger->warn("Too many decals, only the first {} will be rendered", RB_RENDER_MAX_DECALS);
				break;
			}

			DECALS::_viewBounds[i] = rawrbox::ClusteredUtils::toViewSphere(view, DECALS::_decals[i].bounds);
			DECALS::_entries.push_back(rawrbox::ClusteredUtils::getEntry(DECALS::_viewBounds[i], static_cast<uint32_t>(i)));
		}

		rawrbox::ClusteredUtils::build(DECALS::_entries, 0, nearFar.x, nearFar.y, DECALS::_zbins);

		DECALS::_sorted.clear();
		DECALS::_bounds.clear();

		for (const auto& entry : DECALS::_entries) {
			DECALS::_sorted.push_back(DECALS::_decals[entry.index]);
			DECALS::_bounds.push_back(DECALS::_viewBounds[entry.index]);
		}
		// ----

		// BARRIER ----
		auto* context = rawrbox::RENDERER->context();

		if (!DECALS::_sorted.empty()) {
			rawrbox::BarrierUtils::barrier({{DECALS::_buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->UpdateBuffer(DECALS::_buffer, 0, sizeof(rawrbox::Decal) * static_cast<uint64_t>(DECALS::_sorted.size()), DECALS::_sorted.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier({{DECALS::_buffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

			rawrbox::BarrierUtils::barrier({{DECALS::_boundsBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->UpdateBuffer(DECALS::_boundsBuffer, 0, sizeof(rawrbox::Vector4f) * static_cast<uint64_t>(DECALS::_bounds.size()), DECALS::_bounds.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier({{DECALS::_boundsBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		}

		rawrbox::BarrierUtils::barrier({{DECALS::_zbinBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateBuffer(DECALS::_zbinBuffer, 0, sizeof(uint32_t) * static_cast<uint64_t>(DECALS::_zbins.size()), DECALS::_zbins.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{DECALS::_zbinBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ---------

		return true;
	}

	bool DECALS::update(const rawrbox::CameraBase& camera, bool binned) {
		if (DECALS::uniforms == nullptr) RAWRBOX_CRITICAL("Buffer not initialized! Did you call 'init' ?");

		if (DECALS::_BINNED != binned) {
			DECALS::_BINNED = binned;
			DECALS::_CONSTANTS_DIRTY = true;
		}

		bool rebuilt = false;
		if (binned) rebuilt = DECALS::updateBins(camera); // Sort + bin

		DECALS::updateConstants(); // Update constants if dirty
		return rebuilt;
	}

	// UTILS ----
	Diligent::IBufferView* DECALS::getBuffer() { return DECALS::_bufferRead; }
	Diligent::IBufferView* DECALS::getZBinBuffer() { return DECALS::_zbinBufferRead; }
	Diligent::IBufferView* DECALS::getBoundsBuffer() { return DECALS::_boundsBufferRead; }

	const rawrbox::Decal& DECALS::get(size_t indx) {
		if (indx >= DECALS::_decals.size()) RAWRBOX_CRITICAL("Invalid decal index {}", indx);
		return DECALS::_decals[indx];
	}

	size_t DECALS::count() { return DECALS::_decals.size(); }
	// ----

	// DECALS ----
	void DECALS::add(const rawrbox::Decal& decal) {
		DECALS::_decals.push_back(decal);

		rawrbox::__DECALS_DIRTY__ = true;
		DECALS::_CONSTANTS_DIRTY = true;
	}

	bool DECALS::remove(size_t indx) {
		if (indx >= DECALS::_decals.size()) return false;

		DECALS::_decals.erase(DECALS::_decals.begin() + indx);
		rawrbox::__DECALS_DIRTY__ = true;
		DECALS::_CONSTANTS_DIRTY = true;
		return true;
	}

	void DECALS::clear() {
		if (DECALS::_decals.empty()) return;
		DECALS::_decals.clear();

		rawrbox::__DECALS_DIRTY__ = true;
		DECALS::_CONSTANTS_DIRTY = true;
	}
	// ---------
} // namespace rawrbox
