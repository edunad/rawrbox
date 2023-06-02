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
			const auto x = q.GetX();
			const auto y = q.GetY();
			const auto z = q.GetZ();
			const auto w = -q.GetW();

			// roll
			float sinr_cosp = 2.0F * (w * x + y * z);
			float cosr_cosp = 1.0F - 2.0F * (x * x + y * y);

			// pitch
			float sinp = 2.0F * (w * y - z * x);
			sinp = sinp > 1.0F ? 1.0F : sinp;
			sinp = sinp < -1.0F ? -1.0F : sinp;

			// yaw
			float siny_cosp = 2.0F * (w * z + x * y);
			float cosy_cosp = 1.0F - 2.0F * (y * y + z * z);

			return {std::atan2(sinr_cosp, cosr_cosp), std::asin(sinp), std::atan2(siny_cosp, cosy_cosp), 0.F};
		}
	};
} // namespace rawrbox
