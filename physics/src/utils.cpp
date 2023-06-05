
#include <rawrbox/physics/utils.hpp>

namespace rawrbox {
	rawrbox::Vector3f PhysUtils::posToVec(const JPH::Vec3& q) {
		return {q.GetX(), q.GetY(), q.GetZ()};
	}

	rawrbox::Vector4f PhysUtils::quatToVec4(const JPH::Quat& q) {
		return {q.GetX(), q.GetY(), q.GetZ(), q.GetW()};
	}
} // namespace rawrbox
