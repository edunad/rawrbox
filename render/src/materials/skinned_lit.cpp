#include <rawrbox/render/materials/skinned_lit.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_skinned_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_skinned_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialSkinnedLit::setupUniforms() {
		rawrbox::MaterialLit::setupUniforms();
		this->registerUniform("u_bones", bgfx::UniformType::Mat4, rawrbox::MAX_BONES_PER_MODEL);
	}

	void MaterialSkinnedLit::upload() {
		this->setupUniforms();
		rawrbox::RenderUtils::buildShader(model_skinned_shaders, this->_program);
	}

	uint32_t MaterialSkinnedLit::supports() const {
		return rawrbox::MaterialLit::supports() | rawrbox::MaterialFlags::BONES;
	}

	const bgfx::VertexLayout MaterialSkinnedLit::vLayout() const {
		return rawrbox::VertexData::vLayout(true, true);
	}
} // namespace rawrbox
