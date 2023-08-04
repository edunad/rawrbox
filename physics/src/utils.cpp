
#include <rawrbox/physics/utils.hpp>

namespace rawrbox {
	rawrbox::Vector3f PhysUtils::posToVec(const JPH::Vec3& q) {
		return {q.GetX(), q.GetY(), q.GetZ()};
	}

	rawrbox::Vector4f PhysUtils::quatToVec4(const JPH::Quat& q) {
		return {q.GetX(), q.GetY(), q.GetZ(), q.GetW()};
	}

	JPH::Vec3 PhysUtils::vecToPos(const rawrbox::Vector3f& q) {
		return {q.x, q.y, q.z};
	}

	JPH::Quat PhysUtils::vec4ToQuat(const rawrbox::Vector4f& q) {
		return {q.x, q.y, q.z, q.w};
	}
} // namespace rawrbox
