#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	struct Instance {
		rawrbox::Matrix4x4 matrix = {};
		rawrbox::Colorf color = rawrbox::Colors::White();
		rawrbox::Vector4f extraData = {}; // AtlasID, etc..

		Instance() = default;
		Instance(const rawrbox::Matrix4x4& mat, const rawrbox::Colorf& col = rawrbox::Colors::White(), rawrbox::Vector4f data = {}) : matrix(mat), color(col), extraData(data) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float) // Position
			    .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord3, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord4, 4, bgfx::AttribType::Float) // Color
			    .add(bgfx::Attrib::TexCoord5, 4, bgfx::AttribType::Float) // ExtraData
			    .end();
			return l;
		};
	};
} // namespace rawrbox
