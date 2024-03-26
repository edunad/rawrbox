#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	struct Decal {
	public:
		rawrbox::Matrix4x4 worldToLocal = {};
		rawrbox::Vector4u data = {};
		rawrbox::Colorf color = {};

		Decal() = default;
		Decal(const rawrbox::Matrix4x4& _mtx, const rawrbox::TextureBase& _texture, const rawrbox::Colorf& _color, uint32_t _atlas = 0);

		void setTexture(const rawrbox::TextureBase& texture, uint32_t id = 0);
		void setMatrix(const rawrbox::Matrix4x4& mtx);
		void setColor(const rawrbox::Color& cl);
	};
}; // namespace rawrbox
