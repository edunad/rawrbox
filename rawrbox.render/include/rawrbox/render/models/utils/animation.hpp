#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>

namespace rawrbox {
	class AnimationUtils {
	public:
		static rawrbox::Matrix4x4 toMatrix(const ozz::math::Float4x4& mtx);
		static void getTransform(const ozz::math::SoaTransform& transform, size_t indx, rawrbox::Vector3f& pos, rawrbox::Vector4f& rotation, rawrbox::Vector3f& scale);
		static rawrbox::Matrix4x4 toMatrix(const ozz::math::SoaTransform& soa, size_t lane);
	};
} // namespace rawrbox
