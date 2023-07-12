
#include <rawrbox/render/utils/render.hpp>

#include <bit>
#include <stdexcept>

namespace rawrbox {
	// NOLINTBEGIN(*)
	void RenderUtils::buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, shaders[1].name);

		program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-GBUFFER] Failed to create shader");
	}

	void RenderUtils::buildComputeShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle csh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);

		program = bgfx::createProgram(csh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-GBUFFER] Failed to create shader");
	}
	// NOLINTEND(*)
} // namespace rawrbox
