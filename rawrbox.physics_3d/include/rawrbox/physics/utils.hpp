#pragma once
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <Jolt/Jolt.h>

namespace rawrbox {
	class PhysUtils {
	public:
		static rawrbox::Vector3f posToVec(const JPH::Vec3& q);
		static rawrbox::Vector4f quatToVec4(const JPH::Quat& q);

		static JPH::Vec3 vecToPos(const rawrbox::Vector3f& q);
		static JPH::Quat vec4ToQuat(const rawrbox::Vector4f& q);
	};
} // namespace rawrbox
