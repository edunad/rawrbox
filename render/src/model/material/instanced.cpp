#include <rawrbox/render/model/material/instanced.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_instanced_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_instanced_unlit),
    BGFX_EMBEDDED_SHADER(fs_model_instanced_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialInstancedUnlit::upload() {
		this->buildShader(model_instanced_shaders);
	}
} // namespace rawrbox
