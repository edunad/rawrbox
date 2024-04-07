#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector4.hpp>

namespace rawrbox {
	struct Particle {
	public:
		rawrbox::Vector3f position = {};
		float lifeTime = 0.F;

		rawrbox::Vector3f velocity = {};
		float atlasIndex = 0.F;

		rawrbox::Vector2f size = {};
		rawrbox::Vector2f _padding = {};

		rawrbox::Vector3f rotation = {};
		float _padding_2 = 0.F;

		rawrbox::Colorf color = {};
	};
} // namespace rawrbox
