
#include <rawrbox/render/model/material/text_unlit.hpp>

#include <generated/shaders/render/all.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_text3D_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_unlit),
    BGFX_EMBEDDED_SHADER(fs_text_3d_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialText3DUnlit::upload() {
		this->buildShader(model_text3D_shaders);
	}
} // namespace rawrbox
