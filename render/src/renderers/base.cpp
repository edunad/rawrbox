
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <fmt/format.h>

#define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH)

namespace rawrbox {
	RendererBase::~RendererBase() {
		bgfx::discard();
	}

	void RendererBase::init(const rawrbox::Vector2i& size) {
		if (!this->supported()) throw std::runtime_error(fmt::format("[RawrBox-Renderer] Renderer not supported by GPU!"));
		this->resize(size);

		// finish any queued precomputations before rendering the scene
		bgfx::frame();
	}

	void RendererBase::resize(const rawrbox::Vector2i& size) {
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

		// Final Pass ---------------------
		auto prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_WORLD_VIEW;
		bgfx::touch(rawrbox::CURRENT_VIEW_ID); // Make sure we draw on the view

		// Render world ---
		this->worldRender();
		bgfx::setViewTransform(rawrbox::MAIN_WORLD_VIEW, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
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

	// Is it supported by the GPU?
	bool RendererBase::supported() {
		return true;
	}
} // namespace rawrbox
