
#include <rawrbox/render_temp/utils/render.hpp>

#include <stdexcept>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader quad_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_post_base),
    BGFX_EMBEDDED_SHADER(fs_post_final),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

#define BGFX_STATE_DEFAULT_QUAD (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW)

namespace rawrbox {
	bgfx::ProgramHandle RenderUtils::_quadHandle = BGFX_INVALID_HANDLE; // Won't be clean tough, maybe add a shutdown?
	bgfx::UniformHandle RenderUtils::_s_texColor = BGFX_INVALID_HANDLE; // Won't be clean tough, maybe add a shutdown?

	void RenderUtils::drawQUAD(const bgfx::TextureHandle handle, const rawrbox::Vector2i& wSize, bool useQuadProgram, uint64_t flags) {
		// Setup ----
		if (!bgfx::isValid(_quadHandle)) {
			buildShader(quad_shaders, _quadHandle);
		}

		if (!bgfx::isValid(_s_texColor)) {
			_s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		}
		// -------

		auto winSize = wSize.cast<float>();

		bgfx::TransientVertexBuffer tvb = {};
		bgfx::TransientIndexBuffer tib = {};

		std::array<rawrbox::PosUVVertexData, 4> buff;
		auto addVert = [&buff, &winSize](size_t i, const rawrbox::Vector2f& pos, const rawrbox::Vector2f& uv) {
			buff[i] = {
			    // pos
			    {
				((pos.x) / winSize.x * 2 - 1),
				((pos.y) / winSize.y * 2 - 1) * -1,
				1.0F,
			    },

			    // uv
			    {
				uv.x,
				uv.y,
			    }};
		};

		addVert(0, {0, 0}, {0, 0});
		addVert(1, {0, winSize.y}, {0, 1});
		addVert(2, {winSize.x, 0}, {1, 0});
		addVert(3, {winSize.x, winSize.y}, {1, 1});

		std::array<uint16_t, 6> indc = {0, 1, 2,
		    1, 3, 2};

		auto vertSize = static_cast<uint32_t>(buff.size());
		auto indSize = static_cast<uint32_t>(indc.size());

		if (!bgfx::allocTransientBuffers(&tvb, rawrbox::PosUVVertexData::vLayout(), vertSize, &tib, indSize)) return;

		std::memcpy(tvb.data, buff.data(), vertSize * rawrbox::PosUVVertexData::vLayout().getStride());
		std::memcpy(tib.data, indc.data(), indSize * sizeof(uint16_t));

		bgfx::touch(rawrbox::CURRENT_VIEW_ID);
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);

		bgfx::setTexture(0, _s_texColor, handle);
		bgfx::setVertexBuffer(0, &tvb);
		bgfx::setIndexBuffer(&tib);
		bgfx::setState(BGFX_STATE_DEFAULT_QUAD | flags);

		if (useQuadProgram)
			bgfx::submit(rawrbox::CURRENT_VIEW_ID, _quadHandle);
	}

	// NOLINTBEGIN(*)
	void RenderUtils::buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();

		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		if (!bgfx::isValid(vsh)) throw std::runtime_error("[RawrBox-Renderer] Failed to create compute shader");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, shaders[1].name);
		if (!bgfx::isValid(fsh)) throw std::runtime_error("[RawrBox-Renderer] Failed to create compute shader");

		program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-Renderer] Failed to create shader");
	}

	void RenderUtils::buildComputeShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();

		bgfx::ShaderHandle csh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		if (!bgfx::isValid(csh)) throw std::runtime_error("[RawrBox-Renderer] Failed to create compute shader");

		program = bgfx::createProgram(csh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-Renderer] Failed to create compute shader");
	}
	// NOLINTEND(*)
} // namespace rawrbox
