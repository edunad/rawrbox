#include <rawrbox/render/model/material/skinned_lit.hpp>

#include <generated/shaders/render/all.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_skinned_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_skinned_lit),
    BGFX_EMBEDDED_SHADER(fs_model_lit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialSkinnedLit::~MaterialSkinnedLit() {
		RAWRBOX_DESTROY(u_bones);
	}

	void MaterialSkinnedLit::registerUniforms() {
		MaterialLit::registerUniforms();

		// BONES ----
		u_bones = bgfx::createUniform("u_bones", bgfx::UniformType::Mat4, rawrbox::MAX_BONES_PER_MODEL);
		// ---
	}

	void MaterialSkinnedLit::setBoneData(const std::vector<rawrbox::Matrix4x4>& data) {
		bgfx::setUniform(this->u_bones, &data.front(), static_cast<uint32_t>(data.size()));
	}

	void MaterialSkinnedLit::upload() {
		this->buildShader(model_skinned_lit_shaders, "model_skinned_lit");
	}
} // namespace rawrbox
