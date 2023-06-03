#pragma once
#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <Jolt/Jolt.h>

namespace rawrbox {
	class PhysUtils {
	public:
		static rawrbox::Vector3f posToVec(const JPH::Vec3& q) {
			return {q.GetX(), q.GetY(), q.GetZ()};
		}

		static rawrbox::Vector4f quatToVec4(const JPH::Quat& q) {
			return {q.GetX(), q.GetY(), q.GetZ(), q.GetW()};
		}
	};
} // namespace rawrbox
