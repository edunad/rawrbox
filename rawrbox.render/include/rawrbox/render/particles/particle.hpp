#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector4.hpp>

namespace rawrbox {
	struct Particle {
	public:
		rawrbox::Vector3f position = {};
		float lifeTime = 0.F;

		rawrbox::Vector3f velocity = {};
		float initialLifeTime = 0.F;

		rawrbox::Vector2f size = {};
		rawrbox::Vector2f rotation = {};

		rawrbox::Colorf color = {};

		Particle() = default;
	};
} // namespace rawrbox
