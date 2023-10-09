#include <rawrbox/render_temp/materials/instanced_lit.hpp>
#include <rawrbox/render_temp/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_instanced_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_instance_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	uint32_t MaterialInstancedLit::supports() const {
		return rawrbox::MaterialLit::supports() | rawrbox::MaterialFlags::INSTANCED;
	}

	void MaterialInstancedLit::upload() {
		this->setupUniforms();
		rawrbox::RenderUtils::buildShader(model_instanced_lit_shaders, this->_program);
	}
} // namespace rawrbox
