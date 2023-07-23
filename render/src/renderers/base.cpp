
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <fmt/format.h>

#define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH)

namespace rawrbox {
	RendererBase::~RendererBase() {
		rawrbox::DECALS::shutdown();

		RAWRBOX_DESTROY(this->_u_camPos);
		bgfx::discard(BGFX_DISCARD_ALL);
	}

	void RendererBase::init(const rawrbox::Vector2i& size) {
		if (!this->supported()) throw std::runtime_error(fmt::format("[RawrBox-Renderer] Renderer not supported by GPU!"));

		// Setup uniforms ---
		this->_u_camPos = bgfx::createUniform("u_camPos", bgfx::UniformType::Vec4);
		// ----

		rawrbox::DECALS::init();
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
	void RendererBase::overridePostWorld(std::function<void()> post) { this->postRender = post; }

	void RendererBase::render() {
		if (this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");
		if (this->overlayRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Overlay render method not set! Did you call 'setOverlayRender' ?");

		// No world / overlay only
		if (rawrbox::MAIN_CAMERA == nullptr) {
			// Render overlay ---
			auto prevId = rawrbox::CURRENT_VIEW_ID;
			rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_OVERLAY_VIEW;

			// ---
			bgfx::touch(rawrbox::CURRENT_VIEW_ID);
			bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
			this->overlayRender();
			// ----------------

			// Restore id -----
			rawrbox::CURRENT_VIEW_ID = prevId;
			bgfx::discard(BGFX_DISCARD_ALL);
			// ------------------------

			this->frame(); // No camera, prob just stencil?
			return;
		}

		// Final Pass -------------
		this->finalRender();
		// ------------------------

		this->frame(); // Submit ---
	}

	void RendererBase::finalRender() {
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

	void RendererBase::frame() {
		rawrbox::BGFX_FRAME = bgfx::frame();
	}

	void RendererBase::bindRenderUniforms() {}
	void RendererBase::bindCamera() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("[RawrBox-Renderer] Failed to bind camera, MAIN_CAMERA not set!");
		bgfx::setUniform(this->_u_camPos, rawrbox::MAIN_CAMERA->getPos().data().data());
	}

	// Utils ----
	const bgfx::TextureHandle RendererBase::getDepth() const {
		return BGFX_INVALID_HANDLE;
	}

	const bgfx::TextureHandle RendererBase::getColor() const {
		return BGFX_INVALID_HANDLE;
	}

	const bgfx::TextureHandle RendererBase::getMask() const {
		return BGFX_INVALID_HANDLE;
	}
	// ------

	// Is it supported by the GPU?
	bool RendererBase::supported() {
		const bgfx::Caps* caps = bgfx::getCaps();
		return (caps->supported & BGFX_CAPS_INSTANCING) != 0;
	}
} // namespace rawrbox
