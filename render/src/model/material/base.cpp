
#include <rawrbox/render/model/material/base.hpp>

#include <generated/shaders/render/all.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_unlit),
    BGFX_EMBEDDED_SHADER(fs_model_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {

	MaterialBase::~MaterialBase() {
		RAWRBOX_DESTROY(program);

		RAWRBOX_DESTROY(s_texColor);

		RAWRBOX_DESTROY(u_colorOffset);

		RAWRBOX_DESTROY(u_mesh_pos);
		RAWRBOX_DESTROY(u_data);
	}

	// NOLINTBEGIN(hicpp-avoid-c-arrays)
	void MaterialBase::buildShader(const bgfx::EmbeddedShader shaders[], const std::string& name) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, shaders[1].name);

		program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-MaterialBase] Failed to create shader");
	}
	// NOLINTEND(hicpp-avoid-c-arrays)

	void MaterialBase::registerUniforms() {
		s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

		u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);
		u_mesh_pos = bgfx::createUniform("u_mesh_pos", bgfx::UniformType::Vec4, 3);
		u_data = bgfx::createUniform("u_data", bgfx::UniformType::Vec4, 1);
	}

	void MaterialBase::preProcess(const rawrbox::Vector3f& camPos) {}
	void MaterialBase::process(const bgfx::TextureHandle& texture) {
		if (bgfx::isValid(texture)) {
			bgfx::setTexture(0, s_texColor, texture);
		} else {
			bgfx::setTexture(0, s_texColor, rawrbox::WHITE_TEXTURE->getHandle());
		}
	}

	void MaterialBase::postProcess() { bgfx::submit(rawrbox::CURRENT_VIEW_ID, program); }
	void MaterialBase::upload() {
		this->buildShader(model_unlit_shaders, "model_unlit");
	}

} // namespace rawrbox
