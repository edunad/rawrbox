#pragma once

#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_skinned_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_skinned_unlit),
    BGFX_EMBEDDED_SHADER(fs_model_skinned_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {

	class MaterialSkinnedUnlit : public rawrbox::MaterialBase {
	public:
		bgfx::UniformHandle u_bones = BGFX_INVALID_HANDLE;

		using vertexBufferType = rawrbox::VertexSkinnedUnlitData;

		MaterialSkinnedUnlit() = default;
		MaterialSkinnedUnlit(MaterialSkinnedUnlit&&) = delete;
		MaterialSkinnedUnlit& operator=(MaterialSkinnedUnlit&&) = delete;
		MaterialSkinnedUnlit(const MaterialSkinnedUnlit&) = delete;
		MaterialSkinnedUnlit& operator=(const MaterialSkinnedUnlit&) = delete;
		~MaterialSkinnedUnlit() {
			RAWRBOX_DESTROY(u_bones);
		}

		void registerUniforms() {
			MaterialBase::registerUniforms();

			// BONES ----
			u_bones = bgfx::createUniform("u_bones", bgfx::UniformType::Mat4, rawrbox::MAX_BONES_PER_MODEL);
			// ---
		}

		void upload() {
			buildShader(model_skinned_unlit_shaders, "model_skinned_unlit");
		}
	};

} // namespace rawrbox
