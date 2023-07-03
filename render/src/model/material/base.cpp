
#include <rawrbox/render/model/material/base.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_gbuffer_model),
    BGFX_EMBEDDED_SHADER(fs_gbuffer_model),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {

	MaterialBase::~MaterialBase() {
		RAWRBOX_DESTROY(this->program);

		RAWRBOX_DESTROY(this->s_albedo);
		RAWRBOX_DESTROY(this->s_normal);

		// Uniforms -----
		RAWRBOX_DESTROY(this->u_colorOffset);
		RAWRBOX_DESTROY(this->u_mesh_pos);
		RAWRBOX_DESTROY(this->u_data);
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
		this->s_albedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Sampler);
		this->s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler);
		this->s_specular = bgfx::createUniform("s_specular", bgfx::UniformType::Sampler);

		this->u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);
		this->u_mesh_pos = bgfx::createUniform("u_mesh_pos", bgfx::UniformType::Vec4, 3);
		this->u_data = bgfx::createUniform("u_data", bgfx::UniformType::Vec4, 4);
	}

	void MaterialBase::process(const rawrbox::Mesh& mesh) {
		if (mesh.texture != nullptr && mesh.texture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(0, s_albedo, mesh.texture->getHandle());
		} else {
			bgfx::setTexture(0, s_albedo, rawrbox::WHITE_TEXTURE->getHandle());
		}

		if (mesh.normalTexture != nullptr && mesh.normalTexture->valid()) {
			bgfx::setTexture(1, s_normal, mesh.normalTexture->getHandle());
		} else {
			bgfx::setTexture(1, s_normal, rawrbox::NORMAL_TEXTURE->getHandle());
		}

		if (mesh.specularTexture != nullptr && mesh.specularTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(2, s_specular, mesh.specularTexture->getHandle());
		} else {
			bgfx::setTexture(2, s_specular, rawrbox::WHITE_TEXTURE->getHandle());
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

		/*if (mesh.emissionTexture != nullptr && mesh.emissionTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(3, s_emission, mesh.emissionTexture->getHandle());
		} else {
			bgfx::setTexture(3, s_emission, rawrbox::BLACK_TEXTURE->getHandle());
		}

		if (mesh.opacityTexture != nullptr && mesh.opacityTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(4, s_opacity, mesh.opacityTexture->getHandle());
		} else {
			bgfx::setTexture(4, s_opacity, rawrbox::WHITE_TEXTURE->getHandle());
		}

		// Extra material data --
		bgfx::setUniform(u_specularColor, mesh.specularColor.data().data());
		bgfx::setUniform(u_emissionColor, mesh.emissionColor.data().data());

		// std::array<float, 2> matData = {mesh.specularShininess, mesh.emissionIntensity};
		// bgfx::setUniform(u_texMatData, matData.data());
		//  ----
		// ---*/
	}

	void MaterialBase::process(const bgfx::TextureHandle& texture) {
		if (bgfx::isValid(texture)) {
			bgfx::setTexture(0, s_albedo, texture);
		} else {
			bgfx::setTexture(0, s_albedo, rawrbox::WHITE_TEXTURE->getHandle());
		}
	}

	void MaterialBase::postProcess() { bgfx::submit(rawrbox::CURRENT_VIEW_ID, program); }
	void MaterialBase::upload() {
		this->buildShader(model_shaders);
	}

} // namespace rawrbox
