#include <rawrbox/render/model/material/skinned_unlit.hpp>

#include <generated/shaders/render/all.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_skinned_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_skinned_unlit),
    BGFX_EMBEDDED_SHADER(fs_model_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialSkinnedUnlit::~MaterialSkinnedUnlit() {
		RAWRBOX_DESTROY(u_bones);
	}

	void MaterialSkinnedUnlit::registerUniforms() {
		MaterialBase::registerUniforms();

		// BONES ----
		u_bones = bgfx::createUniform("u_bones", bgfx::UniformType::Mat4, rawrbox::MAX_BONES_PER_MODEL);
		// ---
	}

	void MaterialSkinnedUnlit::setBoneData(const std::vector<rawrbox::Matrix4x4>& data) {
		bgfx::setUniform(this->u_bones, &data.front(), static_cast<uint32_t>(data.size()));
	}

	void MaterialSkinnedUnlit::upload() {
		this->buildShader(model_skinned_unlit_shaders);
	}

} // namespace rawrbox
