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
		Decal(const Decal&) = default;
		Decal(Decal&&) = delete;
		Decal& operator=(const Decal&) = default;
		Decal& operator=(Decal&&) = delete;
		virtual ~Decal() = default;

		virtual void setTexture(const rawrbox::TextureBase& texture, uint32_t id = 0);

		virtual void setScale(const rawrbox::Vector3f& scale);
		virtual void setPos(const rawrbox::Vector3f& pos);
		virtual void setColor(const rawrbox::Color& cl);
	};
}; // namespace rawrbox
