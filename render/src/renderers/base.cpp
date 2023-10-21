
// #include <rawrbox/render_temp/decals/manager.hpp>
//
//
// #include <rawrbox/render_temp/utils/render.hpp>
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>
#include <rawrbox/utils/pack.hpp>

#include <fmt/format.h>

// #define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH)

namespace rawrbox {
	RendererBase::RendererBase(const rawrbox::Colorf& clearColor) : _clearColor(clearColor) {}
	RendererBase::~RendererBase() {
		this->_render.reset();

		/*rawrbox::DECALS::shutdown();
		this->_decals.reset();
		bgfx::discard(BGFX_DISCARD_ALL);*/
	}

	void RendererBase::init(const rawrbox::Vector2i& size) {
		if (!this->supported()) throw std::runtime_error(fmt::format("[RawrBox-Renderer] Renderer not supported by GPU!"));
		this->resize(size);

		// Init materials ---
		rawrbox::MaterialBase::init();
		rawrbox::MaterialText3D::init();
		rawrbox::MaterialInstanced::init();
		// -----

		// rawrbox::DECALS::init();
	}

	void RendererBase::resize(const rawrbox::Vector2i& size) {
		if (this->swapChain == nullptr) return;
		this->swapChain->Resize(size.x, size.y);

		this->_render = std::make_unique<rawrbox::TextureRender>(size);
		// this->_render->addTexture(bgfx::TextureFormat::R8);    // Decal stencil
		// this->_render->addTexture(bgfx::TextureFormat::RGBA8); // GPU PICKING
		this->_render->upload(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB);

		/*
				this->_decals = std::make_unique<rawrbox::TextureRender>(size);
				this->_decals->upload();

				this->_GPUBlitTex = bgfx::createTexture2D(8, 8, false, 1, bgfx::TextureFormat::RGBA8, 0 | BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
				bgfx::setName(this->_GPUBlitTex, "RAWRBOX-BLIT-GPU-PICK");

				auto w = static_cast<uint16_t>(size.x);
				auto h = static_cast<uint16_t>(size.y);

				// Setup view ---
				bgfx::setViewName(rawrbox::MAIN_WORLD_VIEW, "RAWRBOX-MAIN-WORLD");
				bgfx::setViewClear(rawrbox::MAIN_WORLD_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 1.0F, 0, 0);
				bgfx::setViewRect(rawrbox::MAIN_WORLD_VIEW, 0, 0, w, h);

				bgfx::setViewName(rawrbox::MAIN_OVERLAY_VIEW, "RAWRBOX-MAIN-OVERLAY");
				bgfx::setViewClear(rawrbox::MAIN_OVERLAY_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0F, 0);
				bgfx::setViewRect(rawrbox::MAIN_OVERLAY_VIEW, 0, 0, w, h);
				// -----*/

		this->_size = size;
	}

	void RendererBase::setWorldRender(std::function<void()> render) { this->worldRender = render; }
	void RendererBase::setOverlayRender(std::function<void()> render) { this->overlayRender = render; }
	void RendererBase::overridePostWorld(std::function<void()> post) { this->postRender = post; }

	void RendererBase::render() {
		if (this->swapChain == nullptr || this->context == nullptr || this->device == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Failed to bind swapChain/context/device! Did you call 'init' ?");

		if (this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");
		if (this->overlayRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Overlay render method not set! Did you call 'setOverlayRender' ?");

		// Clear backbuffer ----
		this->clear();
		// ---------------------

		// No world / overlay only
		if (rawrbox::MAIN_CAMERA == nullptr) {
			this->overlayRender();
			this->frame();
			return;
		}

		// Final Pass -------------
		this->finalRender();
		// ------------------------

		// Check GPU Picking -----
		this->gpuCheck();
		// -------------------

		// Submit ---
		this->frame();
		// ----------
	}

	void RendererBase::gpuCheck() {
		/*if (this->_gpuReadFrame == rawrbox::BGFX_FRAME) {
			std::unordered_map<uint32_t, uint32_t> ids = {};
			const bgfx::Caps* caps = bgfx::getCaps();

			uint32_t max = 0;
			uint32_t id = 0;

			// ----------
			for (uint8_t* x = this->_gpuPixelData.data(); x < this->_gpuPixelData.data() + rawrbox::GPU_PICK_SAMPLE_SIZE;) {
				uint8_t rr = *x++;
				uint8_t gg = *x++;
				uint8_t bb = *x++;
				uint8_t aa = *x++;

				if (aa == 0) continue;
				if (caps->rendererType == bgfx::RendererType::Direct3D9) {
					std::swap(rr, bb);
				}

				uint32_t hashKey = rawrbox::PackUtils::toABGR(rr / 255.F, gg / 255.F, bb / 255.F, 1.F);
				auto& v = ids[hashKey];
				v++;

				if (v > max) {
					max = v;
					id = hashKey;
				}
			}

			// --------------
			for (auto& p : this->_gpuPickCallbacks) {
				p(id);
			}

			this->_gpuPickCallbacks.clear();
		}*/
	}

	void RendererBase::finalRender() {

		// Record world ---
		this->_render->startRecord();
		this->worldRender();
		this->_render->stopRecord();
		// ----------------

		// Render world ---
		if (this->postRender == nullptr) {
			rawrbox::RenderUtils::renderQUAD(this->_render->getHandle());
			// rawrbox::RenderUtils::drawQUAD(this->_decals->getHandle(), this->_size, true, BGFX_STATE_BLEND_ALPHA);
		} else {
			this->postRender();
		}
		// ----------------

		this->overlayRender();

		/*// Record world ---
		this->_render->startRecord();
		this->worldRender();
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
		this->_render->stopRecord();
		// ----------------

		// Record decals ---
		this->_decals->startRecord();
		rawrbox::DECALS::draw();
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
		this->_decals->stopRecord();
		// -------------------

		// Render world ---
		auto prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_WORLD_VIEW;
		// ---

		if (this->postRender == nullptr) {
			rawrbox::RenderUtils::drawQUAD(this->_render->getHandle(), this->_size);
			rawrbox::RenderUtils::drawQUAD(this->_decals->getHandle(), this->_size, true, BGFX_STATE_BLEND_ALPHA);
		} else {
			this->postRender();
		}

		// -----------------

		// Restore id -----
		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);
		// ------------------------

		// Render overlay ---
		prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_OVERLAY_VIEW;
		// ---
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
		this->overlayRender();
		// ----------------

		// Restore id -----
		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);
		// ------------------------*/
	}

	void RendererBase::clear() {
		if (this->swapChain == nullptr) return;

		auto* pRTV = this->swapChain->GetCurrentBackBufferRTV();
		auto* pDSV = this->swapChain->GetDepthBufferDSV();

		// Reset render target
		this->context->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		// Clear the back buffer
		this->context->ClearRenderTarget(pRTV, this->_clearColor.data().data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		this->context->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.F, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void RendererBase::frame() {
		this->swapChain->Present(this->_vsync ? 1 : 0); // Submit
		rawrbox::FRAME++;
	}

	void RendererBase::bindRenderUniforms() {}

	// Utils ----
	Diligent::ITextureView* RendererBase::getDepth() const {
		return this->_render->getDepth();
	}

	Diligent::ITextureView* RendererBase::getColor() const {
		return this->_render->getHandle();
	}

	/*const bgfx::TextureHandle RendererBase::getMask() const {
		if (this->_render == nullptr) return BGFX_INVALID_HANDLE;
		return this->_render->getTexture(1);
	}

	const bgfx::TextureHandle RendererBase::getGPUPick() const {
		if (this->_render == nullptr) return BGFX_INVALID_HANDLE;
		return this->_render->getTexture(2);
	}*/

	const rawrbox::Vector2i RendererBase::getSize() const { return this->_size; }

	bool RendererBase::getVSync() const { return this->_vsync; }
	void RendererBase::setVSync(bool vsync) { this->_vsync = vsync; }

	void RendererBase::gpuPick(const rawrbox::Vector2i& pos, std::function<void(uint32_t)> callback) {
		/*if (this->_render == nullptr || pos.x < 0 || pos.y < 0 || pos.x >= this->_size.x || pos.y >= this->_size.y) return;

		auto tex = this->getGPUPick();
		if (!bgfx::isValid(tex)) return;

		bgfx::blit(rawrbox::BLIT_VIEW, this->_GPUBlitTex, 0, 0, tex, static_cast<uint16_t>(pos.x), static_cast<uint16_t>(pos.y));
		this->_gpuReadFrame = bgfx::readTexture(this->_GPUBlitTex, this->_gpuPixelData.data());
		this->_gpuPickCallbacks.emplace_back(callback);*/
	}
	// ------

	// Is it supported by the GPU?
	bool RendererBase::supported() const {
		// const auto& Features = m_pDevice->GetDeviceInfo().Features;
		return true;
	}
} // namespace rawrbox
