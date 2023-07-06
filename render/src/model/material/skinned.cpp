#include <rawrbox/render/model/material/skinned.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_skinned_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_skinned_lit),
    BGFX_EMBEDDED_SHADER(fs_model),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialSkinned::~MaterialSkinned() {
		RAWRBOX_DESTROY(u_bones);
	}

	void MaterialSkinned::registerUniforms() {
		MaterialBase::registerUniforms();

		// BONES ----
		u_bones = bgfx::createUniform("u_bones", bgfx::UniformType::Mat4, rawrbox::MAX_BONES_PER_MODEL);
		// ---
	}

	void MaterialSkinned::setBoneData(const std::vector<rawrbox::Matrix4x4>& data) {
		bgfx::setUniform(this->u_bones, &data.front(), static_cast<uint32_t>(data.size()));
	}

	void MaterialSkinned::upload() {
		this->buildShader(model_skinned_lit_shaders);
	}
} // namespace rawrbox
