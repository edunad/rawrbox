#pragma once

#include <rawrbox/math/vector2.hpp>

#include <muli/muli.h>

namespace rawrbox {
	class Phys2DUtils {
	public:
		static rawrbox::Vector2f posToVec(const muli::Vec2& q);
		static rawrbox::Vector2f rotationToVec(const muli::Rotation& q);

		static muli::Vec2 vecToPos(const rawrbox::Vector2f& q);
		static muli::Rotation vecToRotation(const rawrbox::Vector2f& q);
	};
} // namespace rawrbox
