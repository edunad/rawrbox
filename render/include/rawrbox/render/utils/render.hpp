#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>

#define BGFX_STATE_DEFAULT_QUAD (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW)

namespace rawrbox {
	struct PosUVVertexData {
		rawrbox::Vector3f pos = {};
		rawrbox::Vector2f uv = {};

		PosUVVertexData() = default;
		PosUVVertexData(const rawrbox::Vector3f& _pos, const rawrbox::Vector2f& _uv) : pos(_pos), uv(_uv) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout layout;
			layout
			    .begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .end();
			return layout;
		}
	};

	struct PosVertexData {
		rawrbox::Vector3f pos = {};

		PosVertexData() = default;
		PosVertexData(const rawrbox::Vector3f& _pos) : pos(_pos) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout layout;
			layout
			    .begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .end();
			return layout;
		}
	};

	class RenderUtils {
		static bgfx::ProgramHandle _quadHandle;
		static bgfx::UniformHandle _s_texColor;

	public:
		static void drawQUAD(const rawrbox::Vector2i& wSize, bool useQuadProgram = true, uint64_t flags = BGFX_STATE_DEFAULT_QUAD);
		static void drawQUAD(const bgfx::TextureHandle handle, const rawrbox::Vector2i& wSize, bool useQuadProgram = true, uint64_t flags = BGFX_STATE_DEFAULT_QUAD);

		// NOLINTBEGIN(*)
		static void buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);
		static void buildComputeShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);
		// NOLINTEND(*)
	};
} // namespace rawrbox
