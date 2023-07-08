
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <fmt/format.h>

#define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH)

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader quad_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_quadtex),
    BGFX_EMBEDDED_SHADER(fs_quadtex),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

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
		bgfx::setViewName(rawrbox::MAIN_DEFAULT_VIEW, "RAWRBOX-MAIN");
		bgfx::setViewClear(rawrbox::MAIN_DEFAULT_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 1.0F, 0, 0);
		bgfx::setViewRect(rawrbox::MAIN_DEFAULT_VIEW, 0, 0, size.x, size.y);
		// -----
	}

	void RendererBase::setWorldRender(std::function<void()> render) { this->worldRender = render; }
	void RendererBase::render() {
		if (this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");

		bgfx::touch(rawrbox::MAIN_DEFAULT_VIEW); // Make sure we draw on the view
		bgfx::setViewClear(rawrbox::MAIN_DEFAULT_VIEW, BGFX_DEFAULT_CLEAR, 1.0F, 0, 0);
		bgfx::setViewTransform(rawrbox::MAIN_DEFAULT_VIEW, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());

		// render the world
		this->worldRender();
		// ----

		this->frame();
	}

	void RendererBase::frame() {
		rawrbox::BGFX_FRAME = bgfx::frame();
		bgfx::discard(BGFX_DISCARD_ALL);
	}

	void RendererBase::bindRenderUniforms() {}

	// Is it supported by the GPU?
	bool RendererBase::supported() {
		const bgfx::Caps* caps = bgfx::getCaps();
		return
		    // SDR color attachment
		    (caps->formats[bgfx::TextureFormat::BGRA8] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0 &&
		    // HDR color attachment
		    (caps->formats[bgfx::TextureFormat::RGBA16F] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0;
	}
} // namespace rawrbox
