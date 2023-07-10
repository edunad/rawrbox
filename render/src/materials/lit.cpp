
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader clustered_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_base),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader clustered_lit_debug[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_debug_clusters),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader clustered_lit_debug_z[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_debug_clusters_z),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialLit::~MaterialLit() {
		RAWRBOX_DESTROY(this->_program);
		RAWRBOX_DESTROY(this->_debug_z_program);
		RAWRBOX_DESTROY(this->_debug_program);

		RAWRBOX_DESTROY(this->_s_normal);
		RAWRBOX_DESTROY(this->_s_specular);
		RAWRBOX_DESTROY(this->_s_emission);
		RAWRBOX_DESTROY(this->_s_opacity);

		RAWRBOX_DESTROY(this->u_texMatData);
	}

	void MaterialLit::registerUniforms() {
		rawrbox::MaterialBase::registerUniforms();

		this->_s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler);
		this->_s_specular = bgfx::createUniform("s_specular", bgfx::UniformType::Sampler);
		this->_s_emission = bgfx::createUniform("s_emission", bgfx::UniformType::Sampler);
		this->_s_opacity = bgfx::createUniform("s_opacity", bgfx::UniformType::Sampler);

		this->u_texMatData = bgfx::createUniform("u_texMatData", bgfx::UniformType::Vec4);
	}

	void MaterialLit::process(const rawrbox::Mesh& mesh) {
		if (mesh.normalTexture != nullptr && mesh.normalTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_NORMAL, this->_s_normal, mesh.normalTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_NORMAL, this->_s_normal, rawrbox::NORMAL_TEXTURE->getHandle());
		}

		if (mesh.specularTexture != nullptr && mesh.specularTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_SPECULAR, this->_s_specular, mesh.specularTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_SPECULAR, this->_s_specular, rawrbox::BLACK_TEXTURE->getHandle());
		}

		if (mesh.emissionTexture != nullptr && mesh.emissionTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_EMISSION, this->_s_emission, mesh.emissionTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_EMISSION, this->_s_emission, rawrbox::BLACK_TEXTURE->getHandle());
		}

		if (mesh.opacityTexture != nullptr && mesh.opacityTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_OPACITY, this->_s_opacity, mesh.opacityTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_OPACITY, this->_s_opacity, rawrbox::WHITE_TEXTURE->getHandle());
		}

		std::array<float, 2> matData = {mesh.specularShininess, mesh.emissionIntensity};
		bgfx::setUniform(this->u_texMatData, matData.data());

		rawrbox::MaterialBase::process(mesh);
	}

	void MaterialLit::postProcess() {
		switch (rawrbox::RENDERER_DEBUG) {
			case DEBUG_OFF:
				bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program, 0, ~BGFX_DISCARD_BINDINGS);
				break;
			case DEBUG_CLUSTER_Z:
				bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_debug_z_program, 0, ~BGFX_DISCARD_BINDINGS);
				break;
			case DEBUG_CLUSTER_COUNT:
				bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_debug_program, 0, ~BGFX_DISCARD_BINDINGS);
				break;
		}
	}

	void MaterialLit::upload() {
		// Load programs ---
		rawrbox::RenderUtils::buildShader(clustered_lit_shaders, this->_program);
		rawrbox::RenderUtils::buildShader(clustered_lit_debug, this->_debug_program);
		rawrbox::RenderUtils::buildShader(clustered_lit_debug_z, this->_debug_z_program);
		// -----
	}

} // namespace rawrbox
