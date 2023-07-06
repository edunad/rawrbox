
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_base),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader clustered_debug_z[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_debug_clusters_z),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader clustered_debug[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_debug_clusters),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialBase::~MaterialBase() {
		RAWRBOX_DESTROY(this->program);
		RAWRBOX_DESTROY(this->debug_program);
		RAWRBOX_DESTROY(this->debug_z_program);

		RAWRBOX_DESTROY(this->s_albedo);
		RAWRBOX_DESTROY(this->s_normal);

		// Uniforms -----
		RAWRBOX_DESTROY(this->u_colorOffset);
		RAWRBOX_DESTROY(this->u_mesh_pos);
		RAWRBOX_DESTROY(this->u_data);
	}

	void MaterialBase::registerUniforms() {
		this->s_albedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Sampler);
		this->s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler);
		this->s_specular = bgfx::createUniform("s_specular", bgfx::UniformType::Sampler);

		this->u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);
		this->u_mesh_pos = bgfx::createUniform("u_mesh_pos", bgfx::UniformType::Vec4);
		this->u_data = bgfx::createUniform("u_data", bgfx::UniformType::Vec4);
	}

	void MaterialBase::process(const rawrbox::Mesh& mesh) {
		if (mesh.texture != nullptr && mesh.texture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, s_albedo, mesh.texture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, s_albedo, rawrbox::WHITE_TEXTURE->getHandle());
		}

		if (mesh.normalTexture != nullptr && mesh.normalTexture->valid()) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_NORMAL, s_normal, mesh.normalTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_NORMAL, s_normal, rawrbox::NORMAL_TEXTURE->getHandle());
		}

		if (mesh.specularTexture != nullptr && mesh.specularTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_SPECULAR, s_specular, mesh.specularTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_SPECULAR, s_specular, rawrbox::WHITE_TEXTURE->getHandle());
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

		// Bind cluster uniforms
		rawrbox::RENDERER->bindRenderUniforms();
	}

	void MaterialBase::process(const bgfx::TextureHandle& texture) {
		if (bgfx::isValid(texture)) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, s_albedo, texture);
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, s_albedo, rawrbox::WHITE_TEXTURE->getHandle());
		}
	}

	void MaterialBase::postProcess() {
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, program);
	}

	void MaterialBase::upload() {
		// Load programs ---
		rawrbox::RenderUtils::buildShader(model_shaders, program);
		rawrbox::RenderUtils::buildShader(clustered_debug, debug_program);
		rawrbox::RenderUtils::buildShader(clustered_debug_z, debug_z_program);
		// -----
	}

} // namespace rawrbox
