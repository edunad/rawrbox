
#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader text3D_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_text_3d),
    BGFX_EMBEDDED_SHADER(fs_text_3d),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialText3D::upload() {
		rawrbox::RenderUtils::buildShader(text3D_shaders, this->program);
	}
} // namespace rawrbox
