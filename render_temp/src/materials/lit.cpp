
#include <rawrbox/render_temp/light/manager.hpp>
#include <rawrbox/render_temp/materials/lit.hpp>
#include <rawrbox/render_temp/utils/render.hpp>

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
	void MaterialLit::setupUniforms() {
		rawrbox::MaterialBase::setupUniforms();

		this->registerUniform("s_normal", bgfx::UniformType::Sampler);
		this->registerUniform("s_specular", bgfx::UniformType::Sampler);
		this->registerUniform("s_emission", bgfx::UniformType::Sampler);

		this->registerUniform("u_camPos", bgfx::UniformType::Vec4);
		this->registerUniform("u_texMatData", bgfx::UniformType::Vec4);
	}

	MaterialLit::~MaterialLit() {
		RAWRBOX_DESTROY(this->_program);
		RAWRBOX_DESTROY(this->_debug_z_program);
		RAWRBOX_DESTROY(this->_debug_program);
	}

	void MaterialLit::process(const rawrbox::Mesh& mesh) {
		if (mesh.normalTexture != nullptr && mesh.normalTexture->isValid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_NORMAL, this->getUniform("s_normal"), mesh.normalTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_NORMAL, this->getUniform("s_normal"), rawrbox::NORMAL_TEXTURE->getHandle());
		}

		if (mesh.specularTexture != nullptr && mesh.specularTexture->isValid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_SPECULAR, this->getUniform("s_specular"), mesh.specularTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_SPECULAR, this->getUniform("s_specular"), rawrbox::BLACK_TEXTURE->getHandle());
		}

		if (mesh.emissionTexture != nullptr && mesh.emissionTexture->isValid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_EMISSION, this->getUniform("s_emission"), mesh.emissionTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_EMISSION, this->getUniform("s_emission"), rawrbox::BLACK_TEXTURE->getHandle());
		}

		std::array<float, 2> matData = {mesh.specularShininess, mesh.emissionIntensity};
		bgfx::setUniform(this->getUniform("u_texMatData"), matData.data());

		// Camera setup
		bgfx::setUniform(this->getUniform("u_camPos"), rawrbox::MAIN_CAMERA->getPos().data().data());
		// -------

		rawrbox::MaterialBase::process(mesh);
	}

	void MaterialLit::postProcess() {
		switch (rawrbox::RENDERER_DEBUG) {
			default:
			case DEBUG_OFF:
				bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
				break;
			case DEBUG_CLUSTER_Z:
				bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_debug_z_program);
				break;
			case DEBUG_CLUSTER_COUNT:
				bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_debug_program);
				break;
		}
	}

	void MaterialLit::upload() {
		this->setupUniforms();

		// Load programs ---
		rawrbox::RenderUtils::buildShader(clustered_lit_shaders, this->_program);
		rawrbox::RenderUtils::buildShader(clustered_lit_debug, this->_debug_program);
		rawrbox::RenderUtils::buildShader(clustered_lit_debug_z, this->_debug_z_program);
		// -----
	}

	uint32_t MaterialLit::supports() const {
		return rawrbox::MaterialBase::supports() | rawrbox::MaterialFlags::NORMALS;
	}

	const bgfx::VertexLayout MaterialLit::vLayout() const {
		return rawrbox::VertexData::vLayout(true);
	}
} // namespace rawrbox
