#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>

#include <cstdint>
#include <optional>

namespace rawrbox {
	class AnimationBlender;

	struct AnimationIKBase {
		rawrbox::Vector3f target = {};
		float weight = 1.F; // 0 -> 1

		std::optional<uint32_t> joint = std::nullopt;
		bool reached = false;

		AnimationIKBase() = default;
		AnimationIKBase(const AnimationIKBase&) = default;
		AnimationIKBase(AnimationIKBase&&) = default;
		AnimationIKBase& operator=(const AnimationIKBase&) = default;
		AnimationIKBase& operator=(AnimationIKBase&&) = default;
		virtual ~AnimationIKBase() = default;

		virtual void transform(const rawrbox::Matrix4x4& mtx);
		virtual bool setup(rawrbox::AnimationBlender& blender, uint32_t index) = 0;
		virtual bool solve(rawrbox::AnimationBlender& blender) = 0;
	};

	// https://guillaumeblanc.github.io/ozz-animation/samples/look_at/
	struct AnimationIKAim : public rawrbox::AnimationIKBase {
		rawrbox::Vector3f forward = {0.F, 0.F, 1.F};
		rawrbox::Vector3f up = {};
		rawrbox::Vector3f offset = {};

		bool setup(rawrbox::AnimationBlender& blender, uint32_t index) override;
		bool solve(rawrbox::AnimationBlender& blender) override;
	};

	// https://guillaumeblanc.github.io/ozz-animation/samples/two_bone_ik/
	struct AnimationIK : public rawrbox::AnimationIKBase {
		rawrbox::Vector3f pole = {};
		rawrbox::Vector3f midAxis = {};

		float soften = 1.F;
		float twist = 0.F;

		std::optional<uint32_t> start = std::nullopt;
		std::optional<uint32_t> mid = std::nullopt;

		void transform(const rawrbox::Matrix4x4& mtx) override;
		bool setup(rawrbox::AnimationBlender& blender, uint32_t index) override;
		bool solve(rawrbox::AnimationBlender& blender) override;
	};
} // namespace rawrbox
