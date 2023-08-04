
#include <rawrbox/physics_2d/utils.hpp>

namespace rawrbox {
	rawrbox::Vector2f Phys2DUtils::posToVec(const muli::Vec2& q) {
		return {q.x, q.y};
	}

	muli::Vec2 Phys2DUtils::vecToPos(const rawrbox::Vector2f& q) {
		return {q.x, q.y};
	}
} // namespace rawrbox
