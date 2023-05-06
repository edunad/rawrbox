#pragma once

#include <rawrbox/render/model/material/lit.hpp>
#include <rawrbox/render/shader_defines.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_skinned_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_skinned_lit),
    BGFX_EMBEDDED_SHADER(fs_model_skinned_lit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {

	class MaterialSkinnedLit : public rawrbox::MaterialLit {
	public:
		bgfx::UniformHandle u_bones = BGFX_INVALID_HANDLE;

		using vertexBufferType = rawrbox::VertexSkinnedLitData;

		MaterialSkinnedLit() = default;
		MaterialSkinnedLit(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit& operator=(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit(const MaterialSkinnedLit&) = delete;
		MaterialSkinnedLit& operator=(const MaterialSkinnedLit&) = delete;
		~MaterialSkinnedLit() override {
			RAWRBOX_DESTROY(u_bones);
		}

		void registerUniforms() {
			MaterialLit::registerUniforms();

			// BONES ----
			u_bones = bgfx::createUniform("u_bones", bgfx::UniformType::Mat4, rawrbox::MAX_BONES_PER_MODEL);
			// ---
		}

		void upload() {
			buildShader(model_skinned_lit_shaders, "model_skinned_lit");
		}
	};
} // namespace rawrbox
