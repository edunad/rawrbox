#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_skinned_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_skinned_unlit_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_unlit_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialSkinned::setupUniforms() {
		rawrbox::MaterialBase::setupUniforms();
		this->registerUniform("u_bones", bgfx::UniformType::Mat4, rawrbox::MAX_BONES_PER_MODEL);
	}

	void MaterialSkinned::upload() {
		this->setupUniforms();
		rawrbox::RenderUtils::buildShader(model_skinned_shaders, this->_program);
	}

	uint32_t MaterialSkinned::supports() const {
		return rawrbox::MaterialBase::supports() | rawrbox::MaterialFlags::BONES;
	}

	const bgfx::VertexLayout MaterialSkinned::vLayout() const {
		return rawrbox::VertexData::vLayout(false, true);
	}
} // namespace rawrbox
