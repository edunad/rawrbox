#pragma once

#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_text3D_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_text_3d_unlit),
    BGFX_EMBEDDED_SHADER(fs_text_3d_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {

	class MaterialText3DUnlit : public rawrbox::MaterialBase {
	public:
		using vertexBufferType = rawrbox::VertexData;
		MaterialText3DUnlit() = default;

		void upload() {
			buildShader(model_text3D_shaders, "text_3d_unlit");
		}
	};

} // namespace rawrbox
