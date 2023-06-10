
#include <rawrbox/render/model/material/displacement.hpp>

#include <generated/shaders/render/all.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_displacement_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_displacement_unlit),
    BGFX_EMBEDDED_SHADER(fs_model_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialDisplacement::~MaterialDisplacement() {
		RAWRBOX_DESTROY(this->s_heightMap);
	}

	void MaterialDisplacement::registerUniforms() {
		MaterialBase::registerUniforms();
		s_heightMap = bgfx::createUniform("s_heightMap", bgfx::UniformType::Sampler);
	}

	void MaterialDisplacement::upload() {
		this->buildShader(model_displacement_shaders);
	}
} // namespace rawrbox
