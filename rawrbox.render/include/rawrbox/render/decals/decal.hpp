#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class Decal {
	public:
		rawrbox::Matrix4x4 localToWorld = {};
		rawrbox::Colorf color = {};

		uint32_t textureID = 0;
		uint32_t textureAtlasIndex = 0;

		Decal() = default;
		Decal(const rawrbox::Matrix4x4& _mtx, const rawrbox::TextureBase& _texture, const rawrbox::Colorf& _color, uint32_t _atlas = 0);

		void setTexture(const rawrbox::TextureBase& texture, uint32_t id = 0);
		void setScale(const rawrbox::Vector3f& scale);
		void setPos(const rawrbox::Vector3f& pos);
		void setColor(const rawrbox::Color& cl);
	};
}; // namespace rawrbox
