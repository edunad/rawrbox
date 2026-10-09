#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>

namespace rawrbox {
	struct TransformBlend {
		rawrbox::Vector3f pos = {};
		rawrbox::Vector4f rotation = {};
		rawrbox::Vector3f scale = {};

		float weight = 0.F;

		void add(const rawrbox::Vector3f& _pos, const rawrbox::Vector4f& _rotation, const rawrbox::Vector3f& _scale, float _weight);
		[[nodiscard]] rawrbox::Matrix4x4 toMatrix() const;
	};

	class AnimationUtils {
	public:
		static rawrbox::Matrix4x4 toMatrix(const ozz::math::Float4x4& mtx);
		static void getTransform(const ozz::math::SoaTransform& transform, size_t indx, rawrbox::Vector3f& pos, rawrbox::Vector4f& rotation, rawrbox::Vector3f& scale);
	};
} // namespace rawrbox
