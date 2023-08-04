
#include <rawrbox/physics_2d/utils.hpp>

namespace rawrbox {
	rawrbox::Vector2f Phys2DUtils::posToVec(const muli::Vec2& q) {
		return {q.x, q.y};
	}

	rawrbox::Vector2f Phys2DUtils::rotationToVec(const muli::Rotation& q) {
		return {q.c, q.s};
	}

	muli::Vec2 Phys2DUtils::vecToPos(const rawrbox::Vector2f& q) {
		return {q.x, q.y};
	}

	muli::Rotation Phys2DUtils::vecToRotation(const rawrbox::Vector2f& q) {
		muli::Rotation r = {};
		r.c = q.x;
		r.s = q.y;

		return r;
	}
} // namespace rawrbox
