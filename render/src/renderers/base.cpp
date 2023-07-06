
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
		RAWRBOX_DESTROY(this->_frameBuffer); // Kill old buffer

		std::array<bgfx::TextureHandle, 2> textures = {};
		const uint64_t flags = 0 | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT |
				       BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

		if (bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::RGBA8, flags | BGFX_TEXTURE_RT)) {
			textures[0] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, bgfx::TextureFormat::RGBA8, flags | BGFX_TEXTURE_RT); // Color texture
		}

		// Generate depth =---
		bgfx::TextureFormat::Enum depthFormat =
		    bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D32, BGFX_TEXTURE_RT_WRITE_ONLY | flags)
			? bgfx::TextureFormat::D32
			: bgfx::TextureFormat::D16;

		textures[1] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, depthFormat, BGFX_TEXTURE_RT_WRITE_ONLY | flags);
		this->_frameBuffer = bgfx::createFrameBuffer(2, textures.data(), true);
		this->_size = size;

		// Setup view ---
		bgfx::setViewName(rawrbox::MAIN_DEFAULT_VIEW, "RAWRBOX-MAIN");
		bgfx::setViewClear(rawrbox::MAIN_DEFAULT_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 1.0F, 0, 0);
		bgfx::setViewRect(rawrbox::MAIN_DEFAULT_VIEW, 0, 0, size.x, size.y);
		// -----
	}

	void RendererBase::setWorldRender(std::function<void()> render) { this->worldRender = render; }
	void RendererBase::render() {
		if (!bgfx::isValid(this->_frameBuffer) || this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");

		bgfx::touch(rawrbox::MAIN_DEFAULT_VIEW); // Make sure we draw on the view
		bgfx::setViewClear(rawrbox::MAIN_DEFAULT_VIEW, BGFX_DEFAULT_CLEAR, 1.0F, 0, 0);
		bgfx::setViewTransform(rawrbox::MAIN_DEFAULT_VIEW, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
		bgfx::setViewFrameBuffer(rawrbox::MAIN_DEFAULT_VIEW, this->_frameBuffer);

		// render the world
		this->worldRender();
		// ----

		this->postRender();
	}

	void RendererBase::postRender() {
		bgfx::setViewFrameBuffer(rawrbox::MAIN_DEFAULT_VIEW, BGFX_INVALID_HANDLE);

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
