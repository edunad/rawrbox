#pragma once

#include <rawrbox/math/vector3.hpp>

#include <bgfx/bgfx.h>

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

	class RenderUtils {
	public:
		static void renderScreenQuad(const rawrbox::Vector2i& screenSize);
	};
} // namespace rawrbox
