
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <fmt/format.h>

#define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH)

namespace rawrbox {
	RendererBase::~RendererBase() {
		RAWRBOX_DESTROY(this->_frameBuffer);
		bgfx::discard();
	}

	void RendererBase::init(const rawrbox::Vector2i& size) {
		if (!this->supported()) throw std::runtime_error(fmt::format("[RawrBox-Renderer] Renderer not supported by GPU!"));
		this->resize(size);

		// finish any queued precomputations before rendering the scene
		bgfx::frame();
	}

	void RendererBase::resize(const rawrbox::Vector2i& size) {
		// Setup depth ---
		RAWRBOX_DESTROY(this->_frameBuffer); // Kill old buffer

		std::array<bgfx::TextureHandle, 2> textures = {};
		const uint64_t flags = 0 | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT |
				       BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

		if (bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::RGBA8, flags | BGFX_TEXTURE_RT)) {
			textures[0] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, bgfx::TextureFormat::RGBA8, flags | BGFX_TEXTURE_RT); // Color texture
		}

		bgfx::TextureFormat::Enum depthFormat =
		    bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D32, BGFX_TEXTURE_RT_WRITE_ONLY | flags)
			? bgfx::TextureFormat::D32
			: bgfx::TextureFormat::D16;

		textures[1] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, depthFormat, BGFX_TEXTURE_RT_WRITE_ONLY | flags);
		this->_frameBuffer = bgfx::createFrameBuffer(2, textures.data(), true);
		// ---------------

		// Setup view ---
		bgfx::setViewName(rawrbox::MAIN_WORLD_VIEW, "RAWRBOX-MAIN-WORLD");
		bgfx::setViewClear(rawrbox::MAIN_WORLD_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 1.0F, 0, 0);
		bgfx::setViewRect(rawrbox::MAIN_WORLD_VIEW, 0, 0, size.x, size.y);

		bgfx::setViewName(rawrbox::MAIN_OVERLAY_VIEW, "RAWRBOX-MAIN-OVERLAY");
		bgfx::setViewClear(rawrbox::MAIN_OVERLAY_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0F, 0);
		bgfx::setViewRect(rawrbox::MAIN_OVERLAY_VIEW, 0, 0, size.x, size.y);
		// -----

		this->_size = size;
	}

	void RendererBase::setWorldRender(std::function<void()> render) { this->worldRender = render; }
	void RendererBase::setOverlayRender(std::function<void()> render) { this->overlayRender = render; }

	void RendererBase::render() {
		if (this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");
		if (this->overlayRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Overlay render method not set! Did you call 'setOverlayRender' ?");
		if (!bgfx::isValid(this->_frameBuffer)) return;

		// Final Pass ---------------------
		auto prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_WORLD_VIEW;

		bgfx::touch(rawrbox::CURRENT_VIEW_ID); // Make sure we draw on the view
		bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, this->_frameBuffer);

		// Render world ---
		this->worldRender();
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
		bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, BGFX_INVALID_HANDLE);
		// ----------------

		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);

		prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_OVERLAY_VIEW;

		// Render overlay ---
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
		this->overlayRender();
		// ----------------

		rawrbox::CURRENT_VIEW_ID = prevId;
		this->frame();
	}

	void RendererBase::frame() {
		rawrbox::BGFX_FRAME = bgfx::frame();
		bgfx::discard(BGFX_DISCARD_ALL);
	}

	void RendererBase::bindRenderUniforms() {}

	// Utils ----
	bgfx::TextureHandle RendererBase::getDepth() {
		if (!bgfx::isValid(this->_frameBuffer)) return BGFX_INVALID_HANDLE;
		return bgfx::getTexture(this->_frameBuffer, 1);
	}

	bgfx::TextureHandle RendererBase::getColor() {
		if (!bgfx::isValid(this->_frameBuffer)) return BGFX_INVALID_HANDLE;
		return bgfx::getTexture(this->_frameBuffer, 0);
	}
	// ------

	// Is it supported by the GPU?
	bool RendererBase::supported() {
		return true;
	}
} // namespace rawrbox
