#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/plugins/shadows.hpp>
#include <rawrbox/render/shadows/manager.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <GraphicsAccessories.hpp>
#include <MapHelper.hpp>

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>

namespace rawrbox {
	namespace {
		const std::array<rawrbox::Vector3f, 6> FACE_FORWARD = {{{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};
		const std::array<rawrbox::Vector3f, 6> FACE_RIGHT = {{{0, 0, -1}, {0, 0, 1}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {-1, 0, 0}}};
		const std::array<rawrbox::Vector3f, 6> FACE_UP = {{{0, 1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1}, {0, 1, 0}, {0, 1, 0}}};

		constexpr float FACE_CORNER_ANGLE = 0.9553166F;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> createBuffer(const char* name, uint32_t stride, uint32_t count, const Diligent::BIND_FLAGS flags, const void* data = nullptr, const Diligent::BUFFER_MODE mode = Diligent::BUFFER_MODE_STRUCTURED) {
			Diligent::BufferDesc desc;
			desc.Name = name;
			desc.Usage = Diligent::USAGE_DEFAULT;
			desc.Mode = mode;
			desc.BindFlags = flags;
			desc.ElementByteStride = stride;
			desc.Size = static_cast<uint64_t>(stride) * count;

			Diligent::BufferData bufferData;
			bufferData.pData = data;
			bufferData.DataSize = data != nullptr ? desc.Size : 0;

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			rawrbox::RENDERER->device()->CreateBuffer(desc, data != nullptr ? &bufferData : nullptr, &buffer);
			if (buffer == nullptr) RAWRBOX_CRITICAL("Failed to create buffer '{}'", name);

			return buffer;
		}
	} // namespace

	ShadowPlugin::ShadowPlugin(const rawrbox::ShadowSettings& settings) : _settings(settings) {
		const uint32_t pageSize = std::max(this->_settings.pageSize, 1U);

		this->_pages = std::max(this->_settings.virtualSize / pageSize, 1U);
		this->_mips = static_cast<uint32_t>(std::bit_width(this->_pages));

		this->_faceEntries = (4U * this->_pages * this->_pages - 1U) / 3U;
		this->_poolPages = this->_settings.poolSize / pageSize;

		this->_totalPages = this->_poolPages * this->_poolPages;
		this->_totalFaces = this->_settings.maxLights * 6U;

		this->_frame = this->_settings.pageCache + 1U;
	}

	ShadowPlugin::~ShadowPlugin() {
		this->_readback.reset();

		// Cleanup ----
		RAWRBOX_DESTROY(this->_constants);
		RAWRBOX_DESTROY(this->_drawConstants);

		RAWRBOX_DESTROY(this->_pageTable);
		RAWRBOX_DESTROY(this->_requests);

		RAWRBOX_DESTROY(this->_physOwner);
		RAWRBOX_DESTROY(this->_physClear);
		RAWRBOX_DESTROY(this->_touched);

		RAWRBOX_DESTROY(this->_freeStack);
		RAWRBOX_DESTROY(this->_allocator);
		RAWRBOX_DESTROY(this->_ages);

		RAWRBOX_DESTROY(this->_clearList);
		RAWRBOX_DESTROY(this->_clearArgs);

		RAWRBOX_DESTROY(this->_invalidationBuffer);
		RAWRBOX_DESTROY(this->_receiverBuffer);
		RAWRBOX_DESTROY(this->_feedbackBuffer);

		RAWRBOX_DESTROY(this->_viewBuffer);
		RAWRBOX_DESTROY(this->_rectBuffer);
		RAWRBOX_DESTROY(this->_batchBuffer);
		RAWRBOX_DESTROY(this->_batchRectBuffer);
		RAWRBOX_DESTROY(this->_faceRectBuffer);
		RAWRBOX_DESTROY(this->_drawRecordBuffer);
		RAWRBOX_DESTROY(this->_drawArgsBuffer);
		RAWRBOX_DESTROY(this->_slotBuffer);

		RAWRBOX_DESTROY(this->_pool);
		RAWRBOX_DESTROY(this->_canvas);

		RAWRBOX_DESTROY(this->_signature);
		RAWRBOX_DESTROY(this->_signatureBind);
		RAWRBOX_DESTROY(this->_markSignature);
		RAWRBOX_DESTROY(this->_markBind);
		RAWRBOX_DESTROY(this->_casterSignature);
		RAWRBOX_DESTROY(this->_casterBind);
		// -------------
	}

	void ShadowPlugin::requirements(Diligent::DeviceFeatures& features) {
		features.MultiViewport = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		features.GeometryShaders = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		features.PixelUAVWritesAndAtomics = Diligent::DEVICE_FEATURE_STATE_ENABLED;
	}

	void ShadowPlugin::initialize(const rawrbox::Vector2u& /*size*/) {
		const auto& settings = this->_settings;

		// VALIDATE -----
		if (!std::has_single_bit(settings.virtualSize) || !std::has_single_bit(settings.pageSize) || !std::has_single_bit(settings.poolSize))
			RAWRBOX_CRITICAL("Invalid ShadowSettings virtualSize! pageSize & poolSize must be power of two");

		if (settings.pageSize > settings.virtualSize || settings.pageSize > settings.poolSize)
			RAWRBOX_CRITICAL("Invalid ShadowSettings pageSize! Cannot be bigger than the virtual & pool size");

		if (this->_pages > 256 || this->_mips > MAX_MIPS)
			RAWRBOX_CRITICAL("Invalid ShadowSettings! Too many shadow pages per face ({}), pageSize needs to be bigger", this->_pages);

		if (settings.virtualSize > 16384)
			RAWRBOX_CRITICAL("Invalid ShadowSettings! virtualSize cannot be bigger than 16384x16384");

		if (settings.normalBias < 0.F || settings.dither < 0.F || settings.normalBias + settings.dither > this->getMaxBias())
			RAWRBOX_CRITICAL("Invalid ShadowSettings! normalBias & dither must be between 0 and {}", this->getMaxBias());

		if (settings.pageCache == 0)
			RAWRBOX_CRITICAL("Invalid ShadowSettings! pageCache must be at least 1!");

		if (settings.maxLights == 0 || settings.maxLights > MAX_SLOTS)
			RAWRBOX_CRITICAL("Invalid ShadowSettings! maxLights must be between 1 and {}", MAX_SLOTS);

		if (settings.clipmapExtent <= 0.F)
			RAWRBOX_CRITICAL("Invalid ShadowSettings! clipmapExtent must be bigger than 0");

		if (this->_totalPages > 65535 || this->_totalPages < this->_totalFaces)
			RAWRBOX_CRITICAL("Invalid ShadowSettings! shadow pool needs to be between {} 65535 pages", this->_totalFaces);
		// --------------

		// Setup buffers ---
		this->_slots.resize(settings.maxLights);
		this->_keepSlots.resize(settings.maxLights);
		this->_slotFrames.resize(settings.maxLights, 0U);
		this->_slotData.resize(settings.maxLights);
		this->_rects.resize(this->_totalFaces, rawrbox::Recti::empty());

		this->_receiverDepth.resize(this->_totalFaces, 0.F);
		this->_facePending.resize(this->_totalFaces, 0U);
		this->_receiverClear.resize(this->_totalFaces, 0U);
		this->_faceRects.resize(this->_totalFaces, rawrbox::Recti::empty());
		this->_batchRects.resize(this->_totalFaces * 2U, EMPTY_PACKED_RECT);

		this->_drawRecords.reserve(MAX_DRAWS);
		this->_drawArgs.reserve(MAX_DRAWS * DRAW_ARGS);
		// -----------

		this->buildBuffers();
		this->buildTextures();
		this->buildSignatures();
	}

	void ShadowPlugin::upload() {
		if (this->_signature == nullptr) RAWRBOX_CRITICAL("Signature not initialized, did you call 'initialize'?");
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Main camera not initialized");

		auto* cluster = rawrbox::RENDERER->getPlugin<rawrbox::ClusteredPlugin>("Clustered");
		if (cluster == nullptr) RAWRBOX_CRITICAL("The shadow plugin requires the `Clustered` renderer plugin");

		// Setup buffers ---
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowConstants")->Set(this->_constants);

		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowPageTableRW")->Set(this->_pageTable->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowRequests")->Set(this->_requests->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowPhysOwner")->Set(this->_physOwner->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowPhysClear")->Set(this->_physClear->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowTouched")->Set(this->_touched->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowFreeStack")->Set(this->_freeStack->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowAllocator")->Set(this->_allocator->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowAges")->Set(this->_ages->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowClearList")->Set(this->_clearList->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowClearArgs")->Set(this->_clearArgs->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowViewsRW")->Set(this->_viewBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowRectsRW")->Set(this->_rectBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowBatches")->Set(this->_batchBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowFeedback")->Set(this->_feedbackBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowDrawArgs")->Set(this->_drawArgsBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowInvalidations")->Set(this->_invalidationBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowBatchRects")->Set(this->_batchRectBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowFaceRects")->Set(this->_faceRectBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowDrawRecords")->Set(this->_drawRecordBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowSlots")->Set(this->_slotBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowPoolRW")->Set(this->_poolWrite);

		this->_signature->CreateShaderResourceBinding(&this->_signatureBind, true);

		// ---------------
		// ---------------

		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Camera")->Set(rawrbox::CameraBase::uniforms);
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowConstants")->Set(this->_constants);
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "LightConstants")->Set(rawrbox::LIGHTS::uniforms);
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Lights")->Set(rawrbox::LIGHTS::getBuffer());
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "LightZBins")->Set(rawrbox::LIGHTS::getZBinBuffer());
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "LightTiles")->Set(cluster->getLightTilesBuffer(true));
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowRequests")->Set(this->_requests->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowReceiverDepth")->Set(this->_receiverBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
		this->_markSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ShadowSlots")->Set(this->_slotBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));

		this->_markSignature->CreateShaderResourceBinding(&this->_markBind, true);

		// ---------------
		// ---------------

		this->_casterSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ShadowViews")->Set(this->_viewBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		this->_casterSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ShadowDraw")->Set(this->_drawConstants);
		this->_casterSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ShadowSlots")->Set(this->_slotBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		this->_casterSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "ShadowPages")->Set(this->_pageTable->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		this->_casterSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "ShadowRects")->Set(this->_rectBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		this->_casterSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ShadowPoolRW")->Set(this->_poolWrite);
		this->_casterSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ShadowTouched")->Set(this->_touched->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));

		this->_casterSignature->CreateShaderResourceBinding(&this->_casterBind, true);
		// ---------------

		this->_readback = std::make_unique<Diligent::GPUCompletionAwaitQueue<rawrbox::ShadowReadback>>(rawrbox::RENDERER->device());
		this->buildPipelines();
	}

	void ShadowPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig) {
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "ShadowPageTable", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "ShadowPool", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "ShadowConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "ShadowSlots", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
	}

	void ShadowPlugin::bindStatic(Diligent::IPipelineResourceSignature& sig) {
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ShadowPageTable")->Set(this->_pageTable->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ShadowPool")->Set(this->_poolRead);
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ShadowConstants")->Set(this->_constants);
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ShadowSlots")->Set(this->_slotBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
	}

	void ShadowPlugin::setEnabled(const bool enabled) {
		if (!enabled) {
			for (const auto& light : rawrbox::LIGHTS::getLights()) {
				light->setShadowSlot(std::nullopt);
			}

			std::ranges::fill(this->_slots, rawrbox::ShadowSlot{});
		}

		rawrbox::RenderPlugin::setEnabled(enabled);
	}

	void ShadowPlugin::update() {
		if (rawrbox::MAIN_CAMERA == nullptr || this->_slots.empty()) return;

		// Sort shadows based on camera distance ---
		const auto& cameraPos = rawrbox::MAIN_CAMERA->getPos();
		const auto cameraForward = rawrbox::MAIN_CAMERA->getForward();
		const auto& proj = rawrbox::MAIN_CAMERA->getProjMtx();

		const float viewAngle = std::atan(std::sqrt(1.F / (proj[0] * proj[0]) + 1.F / (proj[5] * proj[5])));

		this->_lightCandidate.clear();
		for (const auto& light : rawrbox::LIGHTS::getLights()) {
			const auto held = light->getShadowSlot();
			if (held.has_value() && (held.value() >= this->_slots.size() || this->_slots[held.value()].light.lock() != light)) light->setShadowSlot(std::nullopt);

			if (!requestShadow(*light)) continue;

			if (light->getType() == rawrbox::LightType::DIRECTIONAL) {
				this->_lightCandidate.push_back({.light = light, .hidden = false, .priority = -1.F});
				continue;
			}

			const auto offset = light->getWorldPos() - cameraPos;
			const float distance = offset.length();
			const float radius = light->getRadius();

			const bool visible = distance <= radius || offset.dot(cameraForward) >= distance * std::cos(std::min(viewAngle + std::asin(radius / distance), rawrbox::pi<float>));
			const float keep = light->getShadowSlot().has_value() ? SLOT_KEEP : 1.F;

			this->_lightCandidate.push_back({.light = light, .hidden = !visible, .priority = distance / std::max(radius, 1e-4F) * keep});
		}

		const auto winners = std::min(this->_lightCandidate.size(), this->_slots.size());
		std::ranges::partial_sort(this->_lightCandidate, this->_lightCandidate.begin() + static_cast<std::ptrdiff_t>(winners), {}, [](const auto& candidate) {
			return std::pair{candidate.hidden, candidate.priority};
		});
		// -----------

		// CLEANUP SLOTS ---
		std::ranges::fill(this->_keepSlots, false);
		for (size_t i = 0; i < winners; i++) {
			const auto slot = this->_lightCandidate[i].light->getShadowSlot();
			if (slot.has_value()) this->_keepSlots[slot.value()] = true;
		}

		for (uint32_t i = 0; i < this->_slots.size(); i++) {
			auto& slot = this->_slots[i];
			if (this->_keepSlots[i]) continue;

			const auto light = slot.light.lock();
			if (light == nullptr) continue;

			light->setShadowSlot(std::nullopt);

			slot = {};
			this->_slotData[i] = {};
		}

		auto freeSlot = this->_slots.begin();
		for (size_t i = 0; i < winners; i++) {
			const auto& light = this->_lightCandidate[i].light;
			if (light->getShadowSlot().has_value()) continue;

			freeSlot = std::ranges::find_if(freeSlot, this->_slots.end(), [](const auto& slot) { return slot.light.expired(); });
			*freeSlot = {.light = light};

			const auto index = static_cast<uint32_t>(std::distance(this->_slots.begin(), freeSlot));
			this->_slotFrames[index] = this->_frame + 1U;
			this->resetFeedback(index);

			light->setShadowSlot(index);
		}
		// -----------
	}

	void ShadowPlugin::preRender(const rawrbox::CameraBase& camera) {
		if (&camera != rawrbox::MAIN_CAMERA) return;
		if (this->_freeProgram == nullptr) RAWRBOX_CRITICAL("Compute pipelines not initialized, did you call 'upload'");
		this->_frame++;

		this->readFeedback();
		this->updateSlots();
		this->updateClipmaps(camera);
		this->updateCasters();

		this->recordCasters();
		this->updateConstants(camera);

		this->allocatePages();
		this->render();
		this->resolvePages();
	}

	void ShadowPlugin::postRender(const rawrbox::CameraBase& camera) {
		if (&camera != rawrbox::MAIN_CAMERA || this->_activeSlots == 0) return;

		auto* context = rawrbox::RENDERER->context();
		const auto& size = camera.getRenderTarget()->getSize();

		this->_markBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "CameraDepth")->Set(camera.getDepth());

		context->UpdateBuffer(this->_receiverBuffer, 0, this->_activeSlots * 6U * sizeof(uint32_t), this->_receiverClear.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		context->SetPipelineState(this->_markProgram);
		context->CommitShaderResources(this->_markBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		Diligent::DispatchComputeAttribs dispatch;
		dispatch.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(size.x, MARK_THREADS);
		dispatch.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(size.y, MARK_THREADS);
		context->DispatchCompute(dispatch);

#ifdef _DEBUG
		if (this->_debugMode != rawrbox::SHADOW_DEBUG::NONE) this->drawDebug(camera);
#endif
	}

	// FRAME ---
	void ShadowPlugin::readFeedback() {
		// New first ---
		auto staging = this->_readback->GetFirstCompleted();
		if (staging.buffer == nullptr) return;

		for (auto newer = this->_readback->GetFirstCompleted(); newer.buffer != nullptr; newer = this->_readback->GetFirstCompleted()) {
			this->_readback->Recycle(std::move(staging));
			this->_readbackPending--;

			staging = std::move(newer);
		}
		// -----------

		Diligent::MapHelper<uint8_t> mapped(rawrbox::RENDERER->context(), staging.buffer, Diligent::MAP_READ, Diligent::MAP_FLAG_DO_NOT_WAIT);
		const uint8_t* data = mapped;

		if (data != nullptr) {
			uint32_t failed = 0;
			std::memcpy(&failed, data + 2U * sizeof(uint32_t), sizeof(uint32_t));

			const size_t faces = this->_totalFaces * sizeof(uint32_t);
			std::memcpy(this->_receiverDepth.data(), data + ALLOCATOR_ENTRIES * sizeof(uint32_t), faces);
			std::memcpy(this->_facePending.data(), data + ALLOCATOR_ENTRIES * sizeof(uint32_t) + faces, faces);

			std::vector<uint32_t> rects(this->_totalFaces * 4U);
			std::memcpy(rects.data(), data + ALLOCATOR_ENTRIES * sizeof(uint32_t) + faces * 2U, rects.size() * sizeof(uint32_t));
			for (size_t i = 0; i < this->_faceRects.size(); i++) {
				this->_faceRects[i] = {static_cast<int>(rects[i * 4U]), static_cast<int>(rects[i * 4U + 1U]), static_cast<int>(rects[i * 4U + 2U]), static_cast<int>(rects[i * 4U + 3U])};
			}

			// RESET ---
			for (uint32_t slot = 0; slot < this->_slots.size(); slot++) {
				if (this->_slotFrames[slot] >= staging.frame) this->resetFeedback(slot);
			}
			// -----------

			if (failed != 0) this->_logger->warn("Shadow page pool is full, {} skipped! Increase ShadowSettings poolSize / lodbias!", failed);
		}

		mapped.Unmap();

		this->_readback->Recycle(std::move(staging));
		this->_readbackPending--;
	}

	void ShadowPlugin::updateSlots() {
		std::ranges::fill(this->_rects, rawrbox::Recti::empty());
		this->_invalids.clear();

		this->_activeSlots = 0;
		for (uint32_t i = 0; i < this->_slots.size(); i++) {
			auto& slot = this->_slots[i];

			const auto light = slot.light.lock();
			if (light == nullptr || !requestShadow(*light)) {
				if (light != nullptr) light->setShadowSlot(std::nullopt);

				slot = {};
				this->_slotData[i] = {};
				continue;
			}

			const bool directional = light->getType() == rawrbox::LightType::DIRECTIONAL;
			const auto position = directional ? rawrbox::Vector3f{} : light->getWorldPos();

			const float radius = directional ? 1.F : light->getRadius();
			const auto direction = light->getDirection();

			if (slot.position != position || slot.radius != radius || slot.directional != directional || (directional && slot.direction != direction)) {
				slot.position = position;
				slot.radius = radius;
				slot.directional = directional;
				slot.invalid = true;
			}

			slot.direction = direction;
			slot.cosCone = light->getType() == rawrbox::LightType::SPOT ? std::cos(rawrbox::MathUtils::toRad(light->getData().y) / 2.F) : -1.F;

			if (slot.invalid) {
				for (uint32_t face = 0; face < 6; face++) {
					this->_rects[i * 6U + face] = this->getFullRect();
				}

				slot.invalid = false;
			}

			this->_slotData[i].light = {position.x, position.y, position.z, radius};
			this->_slotData[i].params = {light->getShadowLodBias(), directional ? 0.F : 1.F, 0.F, 0.F};
			this->_activeSlots = i + 1;
		}
	}

	void ShadowPlugin::updateCasters() {
		rawrbox::SHADOWS::update();

		for (const auto& bounds : rawrbox::SHADOWS::getInvalid()) {
			for (uint32_t i = 0; i < this->_activeSlots; i++) {
				const auto& slot = this->_slots[i];
				if (slot.light.expired() || !sphereIntersects(bounds, slot)) continue;

				const uint32_t faces = getCasterFaces(bounds, slot);
				for (uint32_t face = 0; face < 6; face++) {
					if (((faces >> face) & 1U) != 0) this->_rects[i * 6U + face].combine(this->getFaceRect(bounds, slot, face));
				}
			}
		}

		rawrbox::SHADOWS::clearInvalid();

		for (uint32_t i = 0; i < this->_activeSlots * 6U; i++) {
			const auto& rect = this->_rects[i];
			if (rect.isEmpty()) continue;

			this->_invalids.emplace_back((i / 6U) | ((i % 6U) << VIEW_FACE_SHIFT), rect.pack());
		}
	}

	void ShadowPlugin::updateClipmaps(const rawrbox::CameraBase& camera) {
		if (this->_activeSlots == 0) return;

		const auto& pos = camera.getPos();
		const int pageSize = static_cast<int>(this->_settings.pageSize);
		const int half = static_cast<int>(this->_settings.virtualSize / 2U);

		for (uint32_t i = 0; i < this->_activeSlots; i++) {
			auto& slot = this->_slots[i];
			auto& clipmap = this->_slotData[i];

			if (slot.light.expired() || !slot.directional) {
				clipmap = {.params = clipmap.params, .light = clipmap.light};
				continue;
			}

			// Light space ---
			const auto& forward = slot.direction;
			const rawrbox::Vector3f reference = std::abs(forward.y) < 0.99F ? rawrbox::Vector3f{0.F, 1.F, 0.F} : rawrbox::Vector3f{1.F, 0.F, 0.F};

			slot.right = reference.cross(forward).normalized();
			slot.up = forward.cross(slot.right);

			const float depth = pos.dot(forward);
			if (std::abs(depth - slot.depthOrigin) > CLIPMAP_REBASE) {
				slot.depthOrigin = depth;
				for (uint32_t level = 0; level < 6; level++) {
					this->_rects[i * 6U + level] = this->getFullRect();
				}
			}
			// -----------

			clipmap.right = {slot.right.x, slot.right.y, slot.right.z, this->_settings.clipmapExtent};
			clipmap.up = {slot.up.x, slot.up.y, slot.up.z, 1.F};
			clipmap.forward = {forward.x, forward.y, forward.z, CLIPMAP_DEPTH - slot.depthOrigin};

			// Pack pages ----
			for (uint32_t level = 0; level < 6; level++) {
				const float texelSize = this->getClipmapTexelSize(level);

				const rawrbox::Vector2i center = {static_cast<int>(std::floor(pos.dot(slot.right) / texelSize)), static_cast<int>(std::floor(pos.dot(slot.up) / texelSize))};
				const rawrbox::Vector2i page = {static_cast<int>(std::floor(static_cast<float>(center.x) / static_cast<float>(pageSize))), static_cast<int>(std::floor(static_cast<float>(center.y) / static_cast<float>(pageSize)))};
				const rawrbox::Vector2i origin = {page.x * pageSize - half, page.y * pageSize - half};

				auto& current = slot.origins[level];
				if (current != origin) {
					const rawrbox::Vector2i pages = {(origin.x - current.x) / pageSize, (origin.y - current.y) / pageSize};
					const int last = static_cast<int>(this->_pages) - 1;
					const uint32_t id = i | (level << VIEW_FACE_SHIFT);

					if (std::abs(pages.x) > last || std::abs(pages.y) > last) {
						this->_invalids.emplace_back(id, this->getFullRect().pack());
					} else {
						if (pages.x > 0) this->_invalids.emplace_back(id, rawrbox::Recti{last + 1 - pages.x, 0, last, last}.pack());
						if (pages.x < 0) this->_invalids.emplace_back(id, rawrbox::Recti{0, 0, -pages.x - 1, last}.pack());
						if (pages.y > 0) this->_invalids.emplace_back(id, rawrbox::Recti{0, last + 1 - pages.y, last, last}.pack());
						if (pages.y < 0) this->_invalids.emplace_back(id, rawrbox::Recti{0, 0, last, -pages.y - 1}.pack());
					}
				}

				current = origin;

				auto& packed = clipmap.origins[level / 2U];
				if ((level & 1U) == 0) {
					packed.x = origin.x;
					packed.y = origin.y;
				} else {
					packed.z = origin.x;
					packed.w = origin.y;
				}
			}

			// -----------
		}

		rawrbox::RENDERER->context()->UpdateBuffer(this->_slotBuffer, 0, this->_activeSlots * sizeof(rawrbox::ShadowSlotData), this->_slotData.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
	// -----------

	void ShadowPlugin::updateConstants(const rawrbox::CameraBase& camera) {
		const auto& size = camera.getRenderTarget()->getSize();

		rawrbox::ShadowConstants constants = {};
		constants.invView = rawrbox::Matrix4x4::mtxTranspose(rawrbox::Matrix4x4::mtxInverse(camera.getViewMtx()));
		constants.settings = {this->_frame, this->_activeSlots, static_cast<uint32_t>(this->_invalids.size()), std::bit_cast<uint32_t>(this->_settings.dither)};
		constants.params = {this->_settings.lodBias, 2.F / (camera.getProjMtx()[5] * static_cast<float>(size.y)), this->_settings.normalBias, this->_settings.depthBias};
		constants.misc = {0U, 0U, this->_settings.budget, static_cast<uint32_t>(this->_drawRecords.size())};

#ifdef _DEBUG
		constants.misc.x = static_cast<uint32_t>(this->_debugMode);
		constants.misc.y = camera.getRenderTarget()->getDepthTextureID();
#endif

		rawrbox::RENDERER->context()->UpdateBuffer(this->_constants, 0, sizeof(rawrbox::ShadowConstants), &constants, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void ShadowPlugin::recordCasters() {
		const bool cameraCut = rawrbox::FRAME <= rawrbox::MAIN_CAMERA->getTeleportRevision() + MOTION_FRAMES;
		const bool cameraMoved = rawrbox::FRAME <= rawrbox::MAIN_CAMERA->getMoveRevision() + MOTION_FRAMES;

		for (uint32_t attempt = 0; attempt < 2; attempt++) {
			this->_batches.clear();
			this->_batchCasters.clear();
			this->_drawRecords.clear();
			this->_drawArgs.clear();
			this->_drawOverflow = false;

			std::ranges::fill(this->_batchRects, EMPTY_PACKED_RECT);

			this->_revision = rawrbox::SHADOWS::revision();

			for (uint32_t i = 0; i < this->_activeSlots; i++) {
				const auto& slot = this->_slots[i];
				if (slot.light.expired()) continue;

				// SPOT CONES ---
				uint32_t cone = 0x3FU;
				const float coneAngle = std::acos(std::clamp(slot.cosCone, -1.F, 1.F)) + FACE_CORNER_ANGLE;

				if (slot.cosCone > -1.F && coneAngle < rawrbox::pi<float>) {
					cone = 0;
					for (uint32_t face = 0; face < 6; face++) {
						if (std::acos(std::clamp(FACE_FORWARD[face].dot(slot.direction), -1.F, 1.F)) <= coneAngle) cone |= 1U << face;
					}
				}
				// -----------

				// CASTER IN RANGE? ---
				this->_slotCasters.clear();
				for (const auto& entry : rawrbox::SHADOWS::getCasters()) {
					if (!entry.caster->canCastShadows() || !sphereIntersects(entry.bounds, slot)) continue;

					const uint32_t faces = this->getCasterFaces(entry.bounds, slot);
					if (faces == 0) continue;

					this->_slotCasters.push_back({entry.caster, entry.caster->getShadowMode(), entry.bounds, faces});
				}
				// -----------

				for (uint32_t face = 0; face < 6; face++) {
					const uint32_t index = i * 6U + face;

					if (((cone >> face) & 1U) == 0) {
						if (this->_facePending[index] != 0) this->_batchRects[this->getBatch(i, face, rawrbox::ShadowMode::STATIC)] = this->getFullRect().pack();
						continue;
					}

					rawrbox::Recti rect = rawrbox::Recti::empty();
					const auto& requested = this->_faceRects[index];

					if (cameraCut || !this->_rects[index].isEmpty()) {
						rect = this->getFullRect();
					} else if ((cameraMoved || this->_facePending[index] != 0) && !requested.isEmpty()) {
						rect = requested.expand(MOTION_MARGIN).clamp(this->getFullRect());
					}

					if (!rect.isEmpty()) this->recordBatch(i, face, rawrbox::ShadowMode::STATIC, rect);
					this->recordBatch(i, face, rawrbox::ShadowMode::DYNAMIC, this->getFullRect());
				}
			}

			if (this->_revision == rawrbox::SHADOWS::revision()) return;
		}
	}
	// -----------

	void ShadowPlugin::recordBatch(uint32_t slot, uint32_t face, rawrbox::ShadowMode mode, const rawrbox::Recti& rect) {
		const rawrbox::ShadowFaceBatch batch = {this->getBatch(slot, face, mode), slot, face, mode, rect, 0};
		const size_t firstCaster = this->_batchCasters.size();
		const size_t firstDraw = this->_drawRecords.size();

		if (mode == rawrbox::ShadowMode::STATIC) this->_batchRects[batch.batch] = rect.pack();

		const auto draw = this->getDraw(batch, false);
		for (const auto& caster : this->_slotCasters) {
			if (caster.mode != mode || ((caster.faces >> face) & 1U) == 0) continue;
			if (!this->getFaceRect(caster.bounds, this->_slots[slot], face).overlaps(rect)) continue;
			if (mode == rawrbox::ShadowMode::DYNAMIC && this->isBehindReceivers(caster.bounds, slot, face)) continue;

			const size_t before = this->_drawRecords.size();
			caster.caster->drawShadow(draw);

			if (this->_drawRecords.size() == before) continue;
			this->_batchCasters.push_back({.caster = caster.caster, .draws = static_cast<uint32_t>(this->_drawRecords.size() - before)});
		}

		if (mode == rawrbox::ShadowMode::STATIC && this->_drawOverflow) this->_batchRects[batch.batch] = EMPTY_PACKED_RECT;
		if (this->_drawRecords.size() == firstDraw) return;

		auto& recorded = this->_batches.emplace_back(batch);
		recorded.casters = this->_batchCasters.size() - firstCaster;
	}

	rawrbox::ShadowDraw ShadowPlugin::getDraw(const rawrbox::ShadowFaceBatch& batch, bool bind) {
		rawrbox::ShadowDraw draw = {};

		draw.views = this->_mips;
		draw.bind = bind;
		draw.isVisible = [this, batch](const rawrbox::BBOX& bounds) {
			const auto& data = this->_slots[batch.slot];
			if (!sphereIntersects(bounds, data) || ((getCasterFaces(bounds, data) >> batch.face) & 1U) == 0) return false;
			if (!this->getFaceRect(bounds, data, batch.face).overlaps(batch.rect)) return false;

			return batch.mode != rawrbox::ShadowMode::DYNAMIC || !this->isBehindReceivers(bounds, batch.slot, batch.face);
		};

		if (bind) {
			draw.setView = [this, first = batch.batch * this->_mips](uint32_t view) { this->bindDrawView(first, view); };
			draw.submit = [this, first = batch.batch * this->_mips](const rawrbox::ShadowSubmit& submit) {
				if (submit.view == rawrbox::ShadowDraw::ALL_VIEWS) this->bindDrawView(first, 0);
				if (this->_drawCursor >= this->_drawLimit) return;

				Diligent::DrawIndexedIndirectAttribs attribs;
				attribs.pAttribsBuffer = this->_drawArgsBuffer;
				attribs.DrawArgsOffset = static_cast<uint64_t>(this->_drawCursor++) * DRAW_ARGS * sizeof(uint32_t);
				attribs.IndexType = Diligent::VT_UINT32;
				attribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
				attribs.AttribsBufferStateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY;

				rawrbox::RENDERER->context()->DrawIndexedIndirect(attribs);
			};
		} else {
			draw.setView = [](uint32_t /*view*/) {};
			draw.submit = [this, batch](const rawrbox::ShadowSubmit& submit) {
				if (this->_drawRecords.size() >= MAX_DRAWS) {
#ifdef _DEBUG
					this->_logger->warn("Too many shadow caster draws! Only drawing {}", MAX_DRAWS);
#endif

					this->_drawOverflow = true;
					return;
				}

				const auto rect = this->getFaceRect(submit.bounds, this->_slots[batch.slot], batch.face);

				this->_drawRecords.push_back({batch.batch, submit.view, submit.instances, rect.pack()});
				this->_drawArgs.insert(this->_drawArgs.end(), {submit.indices, 0U, submit.firstIndex, submit.baseVertex, 0U});
			};
		}

		return draw;
	}

	void ShadowPlugin::allocatePages() {
		auto* context = rawrbox::RENDERER->context();
		const uint32_t entries = rawrbox::MathUtils::divideRound<uint32_t>(this->_activeSlots * this->total(), THREADS);

		if (!this->_invalids.empty()) {
			context->UpdateBuffer(this->_invalidationBuffer, 0, this->_invalids.size() * sizeof(rawrbox::Vector2u), this->_invalids.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			this->dispatch(this->_invalidateProgram, static_cast<uint32_t>(this->_invalids.size()));
		}

		this->dispatch(this->_freeProgram, rawrbox::MathUtils::divideRound<uint32_t>(std::max({this->_totalPages, this->_activeSlots * this->total(), this->_activeSlots * 12U * this->_mips}), THREADS));
		this->dispatch(this->_evictProgram, rawrbox::MathUtils::divideRound<uint32_t>(this->_totalPages, THREADS));
		if (this->_activeSlots > 0) context->UpdateBuffer(this->_batchRectBuffer, 0, this->_activeSlots * 12U * sizeof(uint32_t), this->_batchRects.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		this->dispatch(this->_allocateProgram, entries);

		// Draw pages & record ---
		this->dispatch(this->_selectProgram, entries);

		if (!this->_drawRecords.empty()) {
			context->UpdateBuffer(this->_drawRecordBuffer, 0, this->_drawRecords.size() * sizeof(rawrbox::Vector4u), this->_drawRecords.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->UpdateBuffer(this->_drawArgsBuffer, 0, this->_drawArgs.size() * sizeof(uint32_t), this->_drawArgs.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			this->dispatch(this->_patchProgram, rawrbox::MathUtils::divideRound<uint32_t>(static_cast<uint32_t>(this->_drawRecords.size()), THREADS));
		}
		// ----------

		// Clear ---
		constexpr std::array<uint32_t, 4> args = {0U, 1U, 1U, 0U};
		context->UpdateBuffer(this->_clearArgs, 0, sizeof(args), args.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		this->dispatch(this->_prepareProgram, rawrbox::MathUtils::divideRound<uint32_t>(this->_totalPages, THREADS));

		context->SetPipelineState(this->_clearProgram);
		context->CommitShaderResources(this->_signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		Diligent::DispatchComputeIndirectAttribs attribs;
		attribs.pAttribsBuffer = this->_clearArgs;
		attribs.AttribsBufferStateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
		context->DispatchComputeIndirect(attribs);
		// ----------
	}

	void ShadowPlugin::render() {
		if (this->_batches.empty()) return;
		auto* context = rawrbox::RENDERER->context();

		const std::array<Diligent::StateTransitionDesc, 4> barriers = {{
		    {this->_pageTable, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_viewBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_rectBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_drawArgsBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDIRECT_ARGUMENT, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		}};
		context->TransitionResourceStates(static_cast<uint32_t>(barriers.size()), barriers.data());

		context->SetRenderTargets(1, &this->_canvasRT, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		context->CommitShaderResources(rawrbox::BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
		context->CommitShaderResources(this->_casterBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		// Create viewports ---
		std::array<Diligent::Viewport, MAX_MIPS> viewports = {};
		std::array<Diligent::Rect, MAX_MIPS> scissors = {};

		for (uint32_t mip = 0; mip < this->_mips; mip++) {
			const uint32_t size = this->_settings.virtualSize >> mip;

			viewports[mip] = {0.F, 0.F, static_cast<float>(size), static_cast<float>(size), 0.F, 1.F};
			scissors[mip] = {0, 0, static_cast<int>(size), static_cast<int>(size)};
		}

		context->SetViewports(this->_mips, viewports.data(), this->_settings.virtualSize, this->_settings.virtualSize);
		context->SetScissorRects(this->_mips, scissors.data(), this->_settings.virtualSize, this->_settings.virtualSize);
		// -----------

		this->_drawView = {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
		this->_drawCursor = 0;

		size_t caster = 0;
		for (const auto& batch : this->_batches) {
			const auto draw = this->getDraw(batch, true);
			this->bindDrawView(batch.batch * this->_mips, 0);

			for (const size_t end = caster + batch.casters; caster < end; caster++) {
				if (this->_revision != rawrbox::SHADOWS::revision()) return;

				this->_drawLimit = this->_drawCursor + this->_batchCasters[caster].draws;
				this->_batchCasters[caster].caster->drawShadow(draw);
				this->_drawCursor = this->_drawLimit;
			}
		}
	}

	void ShadowPlugin::resolvePages() {
		auto* context = rawrbox::RENDERER->context();
		if (this->_activeSlots > 0) this->dispatch(this->_revision != rawrbox::SHADOWS::revision() ? this->_discardProgram : this->_resolveProgram, rawrbox::MathUtils::divideRound<uint32_t>(this->_activeSlots * this->total(), THREADS));

		// Read compute data ---
		if (this->_readbackPending < READBACK_FRAMES) {
			const uint64_t faces = this->_totalFaces * sizeof(uint32_t);
			constexpr uint64_t allocator = ALLOCATOR_ENTRIES * sizeof(uint32_t);

			auto staging = this->_readback->GetRecycled();
			if (staging.buffer == nullptr) {
				Diligent::BufferDesc desc;
				desc.Name = "RawrBox::Shadows::Readback";
				desc.Usage = Diligent::USAGE_STAGING;
				desc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;
				desc.Size = allocator + faces * 6U;

				rawrbox::RENDERER->device()->CreateBuffer(desc, nullptr, &staging.buffer);
				if (staging.buffer == nullptr) RAWRBOX_CRITICAL("Failed to create the shadow readback buffer");
			}

			context->CopyBuffer(this->_allocator, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, staging.buffer, 0, allocator, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->CopyBuffer(this->_receiverBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, staging.buffer, allocator, faces, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->CopyBuffer(this->_feedbackBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, staging.buffer, allocator + faces, faces, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->CopyBuffer(this->_faceRectBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, staging.buffer, allocator + faces * 2U, faces * 4U, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			staging.frame = this->_frame;
			this->_readback->Enqueue(context, std::move(staging));
			this->_readbackPending++;
		}
		// -----------

		std::array<Diligent::StateTransitionDesc, 4> barriers = {{
		    {this->_pageTable, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_pool, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_slotBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_constants, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		}};

		context->TransitionResourceStates(static_cast<uint32_t>(barriers.size()), barriers.data());
	}

#ifdef _DEBUG
	void ShadowPlugin::drawDebug(const rawrbox::CameraBase& camera) {
		auto* context = rawrbox::RENDERER->context();

		if (this->_debugProgram == nullptr) {
			auto* cluster = rawrbox::RENDERER->getPlugin<rawrbox::ClusteredPlugin>("Clustered");

			rawrbox::PipeSettings settings;
			settings.pVS = "quad.vsh";
			settings.pPS = "shadow_debug.psh";
			settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			settings.cull = Diligent::CULL_MODE_NONE;
			settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
			settings.depthWrite = false;
			settings.depthFormat = false;
			settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
			settings.macros = cluster->getClusterMacros() + this->getShadowMacros();
			settings.signatures = {rawrbox::BindlessManager::signature};

			this->_debugProgram = rawrbox::PipelineUtils::createPipeline("Shadows::Debug", settings);
		}

		// COLOR, NO DEPTH ---
		auto* target = camera.getColor(true);
		auto* depth = camera.getDepth()->GetTexture();

		rawrbox::BarrierUtils::barrier({
		    {target->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {depth, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});

		context->SetRenderTargets(1, &target, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		context->SetPipelineState(this->_debugProgram);
		context->CommitShaderResources(rawrbox::BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

		Diligent::DrawAttribs draw;
		draw.NumVertices = 4;
		context->Draw(draw);
		// -----------

		// RECORD ---
		auto* backBuffer = rawrbox::RENDERER->swapChain()->GetCurrentBackBufferRTV();
		context->SetRenderTargets(1, &backBuffer, rawrbox::RENDERER->swapChain()->GetDepthBufferDSV(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		rawrbox::BarrierUtils::barrier({
		    {target->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {depth, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_DEPTH_READ, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		// -----------
	}
#endif

	void ShadowPlugin::dispatch(Diligent::IPipelineState* program, uint32_t groups) {
		if (groups == 0) return;
		auto* context = rawrbox::RENDERER->context();

		context->SetPipelineState(program);
		context->CommitShaderResources(this->_signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		Diligent::DispatchComputeAttribs attribs;
		attribs.ThreadGroupCountX = groups;
		context->DispatchCompute(attribs);
	}

	void ShadowPlugin::bindDrawView(uint32_t first, uint32_t offset) {
		if (this->_drawView.x == first && this->_drawView.y == offset) return;
		this->_drawView = {first, offset};

		Diligent::MapHelper<rawrbox::Vector4u> data(rawrbox::RENDERER->context(), this->_drawConstants, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		if (data == nullptr) RAWRBOX_CRITICAL("Failed to map the shadow draw constants buffer!");

		*data = {first, offset, 0U, 0U};
	}
	// ---------

	// LAYOUT ---
	uint32_t ShadowPlugin::total() const { return this->_faceEntries * 6U; }

	uint32_t ShadowPlugin::getBatch(uint32_t slot, uint32_t face, rawrbox::ShadowMode mode) const {
		return (slot * 6U + face) * 2U + (mode == rawrbox::ShadowMode::DYNAMIC ? 1U : 0U);
	}

	bool ShadowPlugin::isBehindReceivers(const rawrbox::BBOX& bounds, uint32_t slot, uint32_t face) const {
		const float receiver = this->_receiverDepth[slot * 6U + face];
		if (receiver <= 0.F || this->_slots[slot].directional) return false;

		const auto& data = this->_slots[slot];
		const rawrbox::Vector3f closest = {
		    std::clamp(data.position.x, bounds.min.x, bounds.max.x),
		    std::clamp(data.position.y, bounds.min.y, bounds.max.y),
		    std::clamp(data.position.z, bounds.min.z, bounds.max.z)};

		return (closest - data.position).length() / data.radius > receiver * RECEIVER_MARGIN;
	}

	void ShadowPlugin::resetFeedback(uint32_t slot) {
		for (uint32_t face = slot * 6U; face < slot * 6U + 6U; face++) {
			this->_receiverDepth[face] = 0.F;
			this->_facePending[face] = 0U;
			this->_faceRects[face] = rawrbox::Recti::empty();
		}
	}

	float ShadowPlugin::getMaxBias() const {
		return static_cast<float>(this->_settings.pageSize) * 0.5F;
	}

	float ShadowPlugin::getClipmapTexelSize(uint32_t level) const {
		return 2.F * this->_settings.clipmapExtent * static_cast<float>(1U << level) / static_cast<float>(this->_settings.virtualSize);
	}

	rawrbox::Recti ShadowPlugin::getFullRect() const {
		const int last = static_cast<int>(this->_pages) - 1;
		return {0, 0, last, last};
	}
	// ---------

	// UTILS ---
	void ShadowPlugin::createSignature(const char* name, uint8_t index, const std::vector<Diligent::PipelineResourceDesc>& resources, Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature>& signature) {
		Diligent::PipelineResourceSignatureDesc desc;
		desc.Name = name;
		desc.BindingIndex = index;
		desc.Resources = resources.data();
		desc.NumResources = static_cast<uint32_t>(resources.size());

		rawrbox::RENDERER->device()->CreatePipelineResourceSignature(desc, &signature);
		if (signature == nullptr) RAWRBOX_CRITICAL("Failed to create signature '{}'", name);
	}

	bool ShadowPlugin::requestShadow(const rawrbox::LightBase& light) {
		const auto type = light.getType();
		if (!light.castShadows() || !light.isActive()) return false;
		if (type == rawrbox::LightType::DIRECTIONAL) return true;

		return light.getRadius() > 0.F && (type == rawrbox::LightType::POINT || type == rawrbox::LightType::SPOT);
	}

	bool ShadowPlugin::sphereIntersects(const rawrbox::BBOX& bounds, const rawrbox::ShadowSlot& slot) {
		if (slot.directional) return true;

		const rawrbox::Vector3f closest = {
		    std::clamp(slot.position.x, bounds.min.x, bounds.max.x),
		    std::clamp(slot.position.y, bounds.min.y, bounds.max.y),
		    std::clamp(slot.position.z, bounds.min.z, bounds.max.z)};

		const auto delta = closest - slot.position;
		return delta.dot(delta) <= slot.radius * slot.radius;
	}

	uint32_t ShadowPlugin::getCasterFaces(const rawrbox::BBOX& bounds, const rawrbox::ShadowSlot& slot) const {
		if (slot.directional) {
			uint32_t levels = 0;
			for (uint32_t level = 0; level < 6; level++) {
				if (!this->getClipmapRect(bounds, slot, level).isEmpty()) levels |= 1U << level;
			}

			return levels;
		}

		const auto min = bounds.min - slot.position;
		const auto max = bounds.max - slot.position;

		const auto closest = [](const float low, const float high) { return low > 0.F ? low : (high < 0.F ? -high : 0.F); };
		const std::array<float, 3> closestAxis = {closest(min.x, max.x), closest(min.y, max.y), closest(min.z, max.z)};
		const std::array<float, 6> forward = {max.x, -min.x, max.y, -min.y, max.z, -min.z};

		uint32_t faces = 0;
		for (uint32_t face = 0; face < 6; face++) {
			const uint32_t axis = face / 2U;
			bool visible = forward[face] > 0.F;

			for (uint32_t other = 0; other < 3 && visible; other++) {
				if (other != axis) visible = closestAxis[other] <= forward[face];
			}

			if (visible) faces |= 1U << face;
		}

		return faces;
	}

	rawrbox::Recti ShadowPlugin::getFaceRect(const rawrbox::BBOX& bounds, const rawrbox::ShadowSlot& slot, const uint32_t face) const {
		if (slot.directional) return this->getClipmapRect(bounds, slot, face);

		rawrbox::Vector2f uvMin = {1.F, 1.F};
		rawrbox::Vector2f uvMax = {0.F, 0.F};

		for (uint32_t corner = 0; corner < 8; corner++) {
			const rawrbox::Vector3f point = {
			    (corner & 1U) != 0 ? bounds.max.x : bounds.min.x,
			    (corner & 2U) != 0 ? bounds.max.y : bounds.min.y,
			    (corner & 4U) != 0 ? bounds.max.z : bounds.min.z};

			const auto dir = point - slot.position;
			const float depth = dir.dot(FACE_FORWARD[face]);
			if (depth <= 0.F || !std::isfinite(depth)) return this->getFullRect();

			const rawrbox::Vector2f uv = {dir.dot(FACE_RIGHT[face]) / depth * 0.5F + 0.5F, 0.5F - dir.dot(FACE_UP[face]) / depth * 0.5F};

			uvMin = {std::min(uvMin.x, uv.x), std::min(uvMin.y, uv.y)};
			uvMax = {std::max(uvMax.x, uv.x), std::max(uvMax.y, uv.y)};
		}

		const float pages = static_cast<float>(this->_pages);
		return {
		    (uvMin * pages).floor().clamp(0.F, pages - 1.F).cast<int>(),
		    (uvMax * pages).floor().clamp(0.F, pages - 1.F).cast<int>()};
	}

	rawrbox::Recti ShadowPlugin::getClipmapRect(const rawrbox::BBOX& bounds, const rawrbox::ShadowSlot& slot, const uint32_t level) const {
		rawrbox::Vector2f low = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
		rawrbox::Vector2f high = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};

		for (uint32_t corner = 0; corner < 8; corner++) {
			const rawrbox::Vector3f point = {
			    (corner & 1U) != 0 ? bounds.max.x : bounds.min.x,
			    (corner & 2U) != 0 ? bounds.max.y : bounds.min.y,
			    (corner & 4U) != 0 ? bounds.max.z : bounds.min.z};

			const rawrbox::Vector2f pos = {point.dot(slot.right), point.dot(slot.up)};
			low = {std::min(low.x, pos.x), std::min(low.y, pos.y)};
			high = {std::max(high.x, pos.x), std::max(high.y, pos.y)};
		}

		if (!std::isfinite(low.x) || !std::isfinite(low.y) || !std::isfinite(high.x) || !std::isfinite(high.y)) return this->getFullRect();

		const float texelSize = this->getClipmapTexelSize(level);
		const float pageSize = static_cast<float>(this->_settings.pageSize);
		const auto origin = slot.origins[level].cast<float>();
		const float pages = static_cast<float>(this->_pages);

		const auto first = ((low / texelSize - origin) / pageSize).floor();
		const auto last = ((high / texelSize - origin) / pageSize).floor();
		if (last.x < 0.F || last.y < 0.F || first.x >= pages || first.y >= pages) return rawrbox::Recti::empty();

		return {first.clamp(0.F, pages - 1.F).cast<int>(), last.clamp(0.F, pages - 1.F).cast<int>()};
	}

	Diligent::ShaderMacroHelper ShadowPlugin::getShadowMacros() {
		Diligent::ShaderMacroHelper macro;

		macro.AddShaderMacro("SHADOW_VIRTUAL_SIZE", this->_settings.virtualSize);
		macro.AddShaderMacro("SHADOW_PAGE_SIZE", this->_settings.pageSize);
		macro.AddShaderMacro("SHADOW_PAGE_CACHE", this->_settings.pageCache);

		macro.AddShaderMacro("SHADOW_PAGES", this->_pages);
		macro.AddShaderMacro("SHADOW_MIPS", this->_mips);
		macro.AddShaderMacro("SHADOW_FACE_ENTRIES", this->_faceEntries);
		macro.AddShaderMacro("SHADOW_LIGHT_ENTRIES", this->total());

		macro.AddShaderMacro("SHADOW_POOL_PAGES", this->_poolPages);
		macro.AddShaderMacro("SHADOW_TOTAL_PAGES", this->_totalPages);

		macro.AddShaderMacro("SHADOW_PLUGIN", true);
		return macro;
	}

	void ShadowPlugin::createCasterPipeline(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, const Diligent::ShaderMacroHelper& helper, bool normals) {
		if (this->_casterSignature == nullptr) RAWRBOX_CRITICAL("Signature not initialized, did you call 'initialize'?");

		rawrbox::PipeSettings settings;
		settings.pVS = "shadow_caster.vsh";
		settings.pGS = "shadow_caster.gsh";
		settings.pPS = "shadow_caster.psh";

		settings.macros = this->getShadowMacros() + helper;
		if (normals) settings.macros.AddShaderMacro("SHADOW_NORMALS", true);

		settings.cull = Diligent::CULL_MODE_NONE;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.depthWrite = false;
		settings.depthFormat = false;

		settings.renderTargets = 1;
		settings.renderTargetFormat = Diligent::TEX_FORMAT_R8_UNORM;
		settings.colorMask = Diligent::COLOR_MASK_NONE;
		settings.blending = {Diligent::BLEND_FACTOR_UNDEFINED, Diligent::BLEND_FACTOR_UNDEFINED};
		settings.viewports = static_cast<uint8_t>(this->_mips);

		settings.scissors = true;
		settings.signatures = {rawrbox::BindlessManager::signature, this->_casterSignature};

		// Drop the normal / tangent attributes, casters only need the position ---
		settings.layout = layout;
		Diligent::ResolveInputLayoutAutoOffsetsAndStrides(settings.layout.data(), static_cast<uint32_t>(settings.layout.size()));

		if (normals) {
			std::erase_if(settings.layout, [](const Diligent::LayoutElement& element) { return element.InputIndex == 2 || element.InputIndex == 3; });
		}
		// -----------

		rawrbox::PipelineUtils::createPipeline(id + "::Shadow", settings);
	}
	// ---------

	// SETTINGS ---
	const rawrbox::ShadowSettings& ShadowPlugin::getSettings() const { return this->_settings; }

	void ShadowPlugin::setLodBias(float bias) { this->_settings.lodBias = bias; }
	float ShadowPlugin::getLodBias() const { return this->_settings.lodBias; }

	void ShadowPlugin::setNormalBias(float bias) { this->_settings.normalBias = std::clamp(bias, 0.F, this->getMaxBias() - this->_settings.dither); }
	float ShadowPlugin::getNormalBias() const { return this->_settings.normalBias; }

	void ShadowPlugin::setDepthBias(float bias) { this->_settings.depthBias = bias; }
	float ShadowPlugin::getDepthBias() const { return this->_settings.depthBias; }

	void ShadowPlugin::setDither(float size) { this->_settings.dither = std::clamp(size, 0.F, this->getMaxBias() - this->_settings.normalBias); }
	float ShadowPlugin::getDither() const { return this->_settings.dither; }

	void ShadowPlugin::setBudget(uint32_t pages) { this->_settings.budget = pages; }
	uint32_t ShadowPlugin::getBudget() const { return this->_settings.budget; }

#ifdef _DEBUG
	void ShadowPlugin::setDebug(rawrbox::SHADOW_DEBUG mode) { this->_debugMode = mode; }
	rawrbox::SHADOW_DEBUG ShadowPlugin::getDebug() const { return this->_debugMode; }
#endif
	// ------------

	// BUILD ---
	void ShadowPlugin::buildBuffers() {
		auto* device = rawrbox::RENDERER->device();

		// Constants ---
		{
			Diligent::BufferDesc desc;
			desc.Name = "RawrBox::Shadows::Constants";
			desc.Usage = Diligent::USAGE_DEFAULT;
			desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			desc.Size = sizeof(rawrbox::ShadowConstants);

			device->CreateBuffer(desc, nullptr, &this->_constants);
			if (this->_constants == nullptr) RAWRBOX_CRITICAL("Failed to create buffer '{}'", desc.Name);
		}

		{
			Diligent::BufferDesc desc;
			desc.Name = "RawrBox::Shadows::Draw";
			desc.Usage = Diligent::USAGE_DYNAMIC;
			desc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			desc.Size = sizeof(rawrbox::Vector4u);

			device->CreateBuffer(desc, nullptr, &this->_drawConstants);
			if (this->_drawConstants == nullptr) RAWRBOX_CRITICAL("Failed to create buffer '{}'", desc.Name);
		}
		// -------------

		// Page table, the coarse mip of every face is always backed by a reserved page ---
		const std::vector<uint32_t> pageTable(this->total() * this->_settings.maxLights, 0U);
		const std::vector<uint32_t> owners(this->_totalPages, PAGE_EMPTY);

		std::vector<uint32_t> freeStack(this->_totalPages, 0U);
		for (uint32_t i = 0; i < this->_totalPages; i++) {
			freeStack[i] = i;
		}

		const std::vector<uint32_t> requests(this->total() * this->_settings.maxLights, 0U);
		const std::vector<uint32_t> touched(this->_totalPages, 0U);
		const std::vector<uint32_t> clear(this->_totalPages, 3U);
		std::vector<uint32_t> allocator(ALLOCATOR_ENTRIES + this->_mips * 4U, 0U);
		allocator[0] = this->_totalPages;
		const std::array<uint32_t, 4> args = {0U, 1U, 1U, 0U};
		const std::vector<uint32_t> ages(this->_settings.pageCache + 1U, 0U);
		// -------------

		const auto rw = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

		this->_pageTable = createBuffer("RawrBox::Shadows::PageTable", sizeof(uint32_t), this->total() * this->_settings.maxLights, rw, pageTable.data());
		this->_requests = createBuffer("RawrBox::Shadows::Requests", sizeof(uint32_t), this->total() * this->_settings.maxLights, rw, requests.data());

		this->_physOwner = createBuffer("RawrBox::Shadows::PhysOwner", sizeof(uint32_t), this->_totalPages, rw, owners.data());
		this->_physClear = createBuffer("RawrBox::Shadows::PhysClear", sizeof(uint32_t), this->_totalPages, rw, clear.data());
		this->_touched = createBuffer("RawrBox::Shadows::Touched", sizeof(uint32_t), this->_totalPages, rw, touched.data());

		this->_freeStack = createBuffer("RawrBox::Shadows::FreeStack", sizeof(uint32_t), this->_totalPages, rw, freeStack.data());
		this->_allocator = createBuffer("RawrBox::Shadows::Allocator", sizeof(uint32_t), static_cast<uint32_t>(allocator.size()), rw, allocator.data());
		this->_ages = createBuffer("RawrBox::Shadows::Ages", sizeof(uint32_t), static_cast<uint32_t>(ages.size()), rw, ages.data());

		this->_clearList = createBuffer("RawrBox::Shadows::ClearList", sizeof(uint32_t), this->_totalPages, rw);
		this->_clearArgs = createBuffer("RawrBox::Shadows::ClearArgs", sizeof(uint32_t), static_cast<uint32_t>(args.size()), Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_INDIRECT_DRAW_ARGS, args.data(), Diligent::BUFFER_MODE_RAW);

		this->_invalidationBuffer = createBuffer("RawrBox::Shadows::Invalidations", sizeof(rawrbox::Vector2u), this->_totalFaces * 3U, Diligent::BIND_SHADER_RESOURCE);
		this->_receiverBuffer = createBuffer("RawrBox::Shadows::ReceiverDepth", sizeof(uint32_t), this->_totalFaces, rw, this->_receiverClear.data());
		this->_feedbackBuffer = createBuffer("RawrBox::Shadows::Feedback", sizeof(uint32_t), this->_totalFaces, rw, this->_facePending.data());

		const std::vector<uint32_t> batches(this->_totalFaces * 4U, 0U);
		this->_viewBuffer = createBuffer("RawrBox::Shadows::Views", sizeof(uint32_t), this->_totalFaces * this->_mips * 2U, rw);
		this->_rectBuffer = createBuffer("RawrBox::Shadows::Rects", sizeof(uint32_t), this->_totalFaces * this->_mips * 8U, rw);
		this->_batchBuffer = createBuffer("RawrBox::Shadows::Batches", sizeof(uint32_t), this->_totalFaces * 4U, Diligent::BIND_UNORDERED_ACCESS, batches.data());
		const std::vector<uint32_t> faceRects(this->_totalFaces * 4U, 0U);
		this->_batchRectBuffer = createBuffer("RawrBox::Shadows::BatchRects", sizeof(uint32_t), this->_totalFaces * 2U, Diligent::BIND_SHADER_RESOURCE, this->_batchRects.data());
		this->_faceRectBuffer = createBuffer("RawrBox::Shadows::FaceRects", sizeof(uint32_t), this->_totalFaces * 4U, rw, faceRects.data());
		this->_drawRecordBuffer = createBuffer("RawrBox::Shadows::DrawRecords", sizeof(rawrbox::Vector4u), MAX_DRAWS, Diligent::BIND_SHADER_RESOURCE);
		this->_drawArgsBuffer = createBuffer("RawrBox::Shadows::DrawArgs", sizeof(uint32_t), MAX_DRAWS * DRAW_ARGS, Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_INDIRECT_DRAW_ARGS, nullptr, Diligent::BUFFER_MODE_RAW);
		this->_slotBuffer = createBuffer("RawrBox::Shadows::Slots", sizeof(rawrbox::ShadowSlotData), this->_settings.maxLights, Diligent::BIND_SHADER_RESOURCE, this->_slotData.data());

		// BARRIER -----
		const std::vector<Diligent::StateTransitionDesc> barriers = {
		    {this->_constants, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_drawConstants, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_pageTable, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_requests, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_physOwner, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_physClear, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_touched, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_freeStack, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_allocator, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_ages, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_clearList, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_clearArgs, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_invalidationBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_receiverBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_feedbackBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_viewBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_rectBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_batchBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_batchRectBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_faceRectBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_drawRecordBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_drawArgsBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_slotBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		};

		rawrbox::RENDERER->context()->TransitionResourceStates(static_cast<uint32_t>(barriers.size()), barriers.data());
		// -----------
	}

	void ShadowPlugin::buildTextures() {
		auto* device = rawrbox::RENDERER->device();

		// Physical pool ---
		{
			Diligent::TextureDesc desc;
			desc.Name = "RawrBox::Shadows::Pool";
			desc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
			desc.Width = this->_settings.poolSize;
			desc.Height = this->_settings.poolSize;
			desc.ArraySize = 2;
			desc.MipLevels = 1;
			desc.Format = Diligent::TEX_FORMAT_R32_UINT;
			desc.Usage = Diligent::USAGE_DEFAULT;
			desc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;

			device->CreateTexture(desc, nullptr, &this->_pool);
			if (this->_pool == nullptr) RAWRBOX_CRITICAL("Failed to create the shadow pool");

			this->_poolRead = this->_pool->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
			this->_poolWrite = this->_pool->GetDefaultView(Diligent::TEXTURE_VIEW_UNORDERED_ACCESS);
		}
		// ---------

		// Canvas, only used to rasterize the shadow views ---
		{
			Diligent::TextureDesc desc;
			desc.Name = "RawrBox::Shadows::Canvas";
			desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			desc.Width = this->_settings.virtualSize;
			desc.Height = this->_settings.virtualSize;
			desc.MipLevels = 1;
			desc.Format = Diligent::TEX_FORMAT_R8_UNORM;
			desc.Usage = Diligent::USAGE_DEFAULT;
			desc.BindFlags = Diligent::BIND_RENDER_TARGET;

			device->CreateTexture(desc, nullptr, &this->_canvas);
			if (this->_canvas == nullptr) RAWRBOX_CRITICAL("Failed to create the shadow canvas");

			this->_canvasRT = this->_canvas->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
		}
		// ---------

		// BARRIER -----
		const std::vector<Diligent::StateTransitionDesc> barriers = {
		    {this->_pool, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_canvas, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		};

		rawrbox::RENDERER->context()->TransitionResourceStates(static_cast<uint32_t>(barriers.size()), barriers.data());
		// -----------
	}

	void ShadowPlugin::buildSignatures() {
		if (this->_signature != nullptr || this->_markSignature != nullptr || this->_casterSignature != nullptr) RAWRBOX_CRITICAL("Signatures already bound!");

		const auto buffer = Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS;
		const auto stat = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		// Bookkeeping ---
		createSignature("RawrBox::SIGNATURE::Shadows", 0,
		    {
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, stat, buffer},

			{Diligent::SHADER_TYPE_COMPUTE, "ShadowPageTableRW", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowRequests", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowPhysOwner", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowPhysClear", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowTouched", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowFreeStack", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowAllocator", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowAges", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowClearList", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowClearArgs", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowViewsRW", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowRectsRW", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowBatches", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowFeedback", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowFaceRects", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowDrawArgs", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},

			{Diligent::SHADER_TYPE_COMPUTE, "ShadowInvalidations", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowBatchRects", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowDrawRecords", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowSlots", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},

			{Diligent::SHADER_TYPE_COMPUTE, "ShadowPoolRW", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_UAV, stat},
		    },
		    this->_signature);
		// ---------------

		// Marking ---
		createSignature("RawrBox::SIGNATURE::Shadows::Mark", 0,
		    {
			{Diligent::SHADER_TYPE_COMPUTE, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "SCamera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "LightConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, stat, buffer},

			{Diligent::SHADER_TYPE_COMPUTE, "Lights", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "LightTiles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "LightZBins", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "CameraDepth", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},

			{Diligent::SHADER_TYPE_COMPUTE, "ShadowRequests", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowReceiverDepth", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
			{Diligent::SHADER_TYPE_COMPUTE, "ShadowSlots", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
		    },
		    this->_markSignature);
		// ---------------

		// Casters, used alongside the bindless signature ---
		createSignature("RawrBox::SIGNATURE::Shadows::Caster", 1,
		    {
			{Diligent::SHADER_TYPE_VERTEX, "ShadowViews", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
			{Diligent::SHADER_TYPE_VERTEX, "ShadowDraw", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, stat},
			{Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_GEOMETRY, "ShadowSlots", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},

			{Diligent::SHADER_TYPE_GEOMETRY, "ShadowPages", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},
			{Diligent::SHADER_TYPE_GEOMETRY, "ShadowRects", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, stat, buffer},

			{Diligent::SHADER_TYPE_PIXEL, "ShadowPoolRW", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_UAV, stat},
			{Diligent::SHADER_TYPE_PIXEL, "ShadowTouched", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, stat, buffer},
		    },
		    this->_casterSignature);
		// ---------------
	}

	void ShadowPlugin::buildPipelines() {
		auto* cluster = rawrbox::RENDERER->getPlugin<rawrbox::ClusteredPlugin>("Clustered");
		if (cluster == nullptr) RAWRBOX_CRITICAL("The shadow plugin requires the `Clustered` renderer plugin");

		rawrbox::PipeComputeSettings settings;
		settings.macros = cluster->getClusterMacros() + this->getShadowMacros();
		settings.signatures = {this->_signature};

		settings.pCS = "shadow_pages.csh";

		settings.entry = "Invalidate";
		this->_invalidateProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Invalidate", settings);

		settings.entry = "Free";
		this->_freeProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Free", settings);

		settings.entry = "Evict";
		this->_evictProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Evict", settings);

		settings.entry = "Allocate";
		this->_allocateProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Allocate", settings);

		settings.entry = "Select";
		this->_selectProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Select", settings);

		settings.entry = "Patch";
		this->_patchProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Patch", settings);

		settings.entry = "Prepare";
		this->_prepareProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Prepare", settings);

		settings.entry = "Clear";
		this->_clearProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Clear", settings);

		settings.entry = "Resolve";
		this->_resolveProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Resolve", settings);

		settings.entry = "Discard";
		this->_discardProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Discard", settings);

		settings.pCS = "shadow_mark.csh";
		settings.entry = "main";
		settings.signatures = {this->_markSignature};
		this->_markProgram = rawrbox::PipelineUtils::createComputePipeline("Shadows::Mark", settings);
	}
	// ---------

	std::string ShadowPlugin::getID() { return "Shadows"; }
} // namespace rawrbox