#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_instanced_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_instance_unlit_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_unlit_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialInstanced::upload() {
		rawrbox::RenderUtils::buildShader(model_instanced_shaders, this->_program);
	}
} // namespace rawrbox
