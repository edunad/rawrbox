
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_base),
    BGFX_EMBEDDED_SHADER(fs_model_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialLit::~MaterialLit() {
		RAWRBOX_DESTROY(this->_program);

		RAWRBOX_DESTROY(this->_s_normal);
		RAWRBOX_DESTROY(this->_s_specular);
		RAWRBOX_DESTROY(this->_s_emission);

		RAWRBOX_DESTROY(this->u_texMatData);
	}

	void MaterialLit::registerUniforms() {
		rawrbox::MaterialBase::registerUniforms();

		this->_s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler);
		this->_s_specular = bgfx::createUniform("s_specular", bgfx::UniformType::Sampler);
		this->_s_emission = bgfx::createUniform("s_emission", bgfx::UniformType::Sampler);

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

		std::array<float, 2> matData = {mesh.specularShininess, mesh.emissionIntensity};
		bgfx::setUniform(this->u_texMatData, matData.data());

		rawrbox::MaterialBase::process(mesh);
	}

	void MaterialLit::postProcess() {
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
	}

	void MaterialLit::upload() {
		// Load programs ---
		rawrbox::RenderUtils::buildShader(lit_shaders, this->_program);
		// -----
	}

} // namespace rawrbox
