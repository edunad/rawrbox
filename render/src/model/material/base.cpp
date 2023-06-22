
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
		RAWRBOX_DESTROY(s_texBumpColor);

		RAWRBOX_DESTROY(u_colorOffset);

		RAWRBOX_DESTROY(u_mesh_pos);
		RAWRBOX_DESTROY(u_data);
	}

	// NOLINTBEGIN(hicpp-avoid-c-arrays)
	void MaterialBase::buildShader(const bgfx::EmbeddedShader shaders[]) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, shaders[1].name);

		program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-MaterialBase] Failed to create shader");
	}
	// NOLINTEND(hicpp-avoid-c-arrays)

	void MaterialBase::registerUniforms() {
		s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		s_texBumpColor = bgfx::createUniform("s_texBumpColor", bgfx::UniformType::Sampler);

		u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);
		u_mesh_pos = bgfx::createUniform("u_mesh_pos", bgfx::UniformType::Vec4, 3);
		u_data = bgfx::createUniform("u_data", bgfx::UniformType::Vec4, 4);
	}

	void MaterialBase::preProcess() {}

	void MaterialBase::process(const rawrbox::Mesh& mesh) {
		if (mesh.texture != nullptr && mesh.texture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(0, s_texColor, mesh.texture->getHandle());
		} else {
			bgfx::setTexture(0, s_texColor, rawrbox::WHITE_TEXTURE->getHandle());
		}

		if (mesh.bumpTexture != nullptr && mesh.bumpTexture->valid()) {
			bgfx::setTexture(1, s_texBumpColor, mesh.bumpTexture->getHandle());
		} else {
			bgfx::setTexture(1, s_texBumpColor, rawrbox::BLACK_TEXTURE->getHandle());
		}

		// Color override
		bgfx::setUniform(u_colorOffset, mesh.color.data().data());
		// -------

		// Mesh pos
		std::array offset = {mesh.vertexPos[12], mesh.vertexPos[13], mesh.vertexPos[14]};
		bgfx::setUniform(u_mesh_pos, offset.data());
		// -------

		// Pass "special" data ---
		std::array<std::array<float, 4>, 4> data = {std::array<float, 4>{0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}};
		if (mesh.hasData("billboard_mode")) {
			data[0] = mesh.getData("billboard_mode").data();
		}

		if (mesh.hasData("vertex_snap")) {
			data[1] = mesh.getData("vertex_snap").data();
		}

		if (mesh.hasData("displacement_strength")) {
			data[2] = mesh.getData("displacement_strength").data();
		}

		bgfx::setUniform(u_data, data.front().data(), 4);
		// ---
	}

	void MaterialBase::process(const bgfx::TextureHandle& texture) {
		if (bgfx::isValid(texture)) {
			bgfx::setTexture(0, s_texColor, texture);
		} else {
			bgfx::setTexture(0, s_texColor, rawrbox::WHITE_TEXTURE->getHandle());
		}
	}

	void MaterialBase::postProcess() { bgfx::submit(rawrbox::CURRENT_VIEW_ID, program); }
	void MaterialBase::upload() {
		this->buildShader(model_unlit_shaders);
	}

} // namespace rawrbox
