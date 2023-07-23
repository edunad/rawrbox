#include <rawrbox/render/materials/instanced_lit.hpp>
#include <rawrbox/render/utils/render.hpp>

/*
// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_instanced_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_instance_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)
*/
namespace rawrbox {
	void MaterialInstancedLit::upload() {
		// rawrbox::RenderUtils::buildShader(model_instanced_lit_shaders, this->_program);
	}
} // namespace rawrbox
