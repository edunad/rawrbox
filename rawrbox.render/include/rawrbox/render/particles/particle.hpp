#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector4.hpp>

namespace rawrbox {
	struct Particle {
	public:
		rawrbox::Vector4f position = {};
		rawrbox::Vector4f velocity = {};
		rawrbox::Colorf color = {};
		rawrbox::Vector4f data = {};

		Particle() = default;
		Particle(const rawrbox::Vector3f& _position, const rawrbox::Vector3f& _velocity, const rawrbox::Colorf& _color, float life) : position(_position), velocity(_velocity), color(_color), data(life, 0, 0, 0) {}
	};
} // namespace rawrbox
