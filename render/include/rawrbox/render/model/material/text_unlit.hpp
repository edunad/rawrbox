#pragma once

#include <rawrbox/math/quaternion.hpp>
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

namespace rawrBox {

	class MaterialText3DUnlit : public rawrBox::MaterialBase {
	public:
		using vertexBufferType = rawrBox::VertexData;

		MaterialText3DUnlit() = default;
		MaterialText3DUnlit(MaterialText3DUnlit&&) = delete;
		MaterialText3DUnlit& operator=(MaterialText3DUnlit&&) = delete;
		MaterialText3DUnlit(const MaterialText3DUnlit&) = delete;
		MaterialText3DUnlit& operator=(const MaterialText3DUnlit&) = delete;
		~MaterialText3DUnlit() = default;

		void upload() {
			buildShader(model_text3D_shaders, "text_3d_unlit");
		}
	};

} // namespace rawrBox
