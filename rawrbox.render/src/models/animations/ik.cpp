#include <rawrbox/render/models/animations/blender.hpp>
#include <rawrbox/render/models/animations/ik.hpp>

#include <ozz/animation/runtime/ik_aim_job.h>
#include <ozz/animation/runtime/ik_two_bone_job.h>
#include <ozz/base/maths/simd_quaternion.h>

#include <algorithm>

namespace rawrbox {
	// BASE ---
	void AnimationIKBase::transform(const rawrbox::Matrix4x4& mtx) {
		this->target = mtx.mulVec(this->target);
	}
	// --------

	// AIM ---
	bool AnimationIKAim::setup(rawrbox::AnimationBlender& blender, uint32_t index) {
		if (this->forward == rawrbox::Vector3f::zero()) return false;

		this->forward = this->forward.normalized();
		this->joint = index;

		if (this->up == rawrbox::Vector3f()) {
			ozz::math::SimdInt4 invertible = ozz::math::simd_int4::zero();
			const auto inverse = ozz::math::Invert(blender.getRestModel(index), &invertible);
			if (!ozz::math::AreAllTrue1(invertible)) return false;

			const auto rest = ozz::math::NormalizeSafe3(ozz::math::TransformVector(inverse, ozz::math::simd_float4::y_axis()), ozz::math::simd_float4::y_axis());
			this->up = {ozz::math::GetX(rest), ozz::math::GetY(rest), ozz::math::GetZ(rest)};
		} else {
			this->up = this->up.normalized();
		}

		return true;
	}

	bool AnimationIKAim::solve(rawrbox::AnimationBlender& blender) {
		if (!this->joint.has_value()) return false;

		ozz::animation::IKAimJob job;
		job.target = ozz::math::simd_float4::Load3PtrU(&this->target.x);
		job.forward = ozz::math::simd_float4::Load3PtrU(&this->forward.x);
		job.up = ozz::math::simd_float4::Load3PtrU(&this->up.x);
		job.offset = ozz::math::simd_float4::Load3PtrU(&this->offset.x);
		job.weight = this->weight;
		job.reached = &this->reached;

		job.joint = &blender.getModel(this->joint.value());

		ozz::math::SimdQuaternion correction = ozz::math::SimdQuaternion::identity();
		job.joint_correction = &correction;

		if (!job.Run()) return false;

		blender.rotateJoint(this->joint.value(), correction);
		return blender.updateModels(this->joint.value());
	}
	// -------

	// TWO BONE ---
	void AnimationIK::transform(const rawrbox::Matrix4x4& mtx) {
		rawrbox::AnimationIKBase::transform(mtx);
		if (this->pole != rawrbox::Vector3f()) this->pole = mtx.mulVec(this->pole) - mtx.mulVec(rawrbox::Vector3f());
	}

	bool AnimationIK::setup(rawrbox::AnimationBlender& blender, uint32_t index) {
		const auto parents = blender.getSkeleton()->getSkeleton().joint_parents();

		const int midParent = parents[index];
		if (midParent < 0) return false;

		const int startParent = parents[midParent];
		if (startParent < 0) return false;

		this->joint = index;
		this->mid = static_cast<uint32_t>(midParent);
		this->start = static_cast<uint32_t>(startParent);

		const ozz::math::SimdFloat4 startPos = blender.getRestModel(this->start.value()).cols[3];
		const ozz::math::SimdFloat4 midPos = blender.getRestModel(this->mid.value()).cols[3];
		const ozz::math::SimdFloat4 endPos = blender.getRestModel(this->joint.value()).cols[3];

		const ozz::math::SimdFloat4 upper = midPos - startPos;
		const ozz::math::SimdFloat4 lower = endPos - midPos;
		const ozz::math::SimdFloat4 limb = ozz::math::NormalizeSafe3(endPos - startPos, ozz::math::simd_float4::y_axis());

		const ozz::math::SimdFloat4 bendAxis = ozz::math::Cross3(lower, upper);
		const float bendSin = ozz::math::GetX(ozz::math::Length3(bendAxis)) / std::max(ozz::math::GetX(ozz::math::Length3(upper)) * ozz::math::GetX(ozz::math::Length3(lower)), 0.000001F);
		const bool bent = bendSin > 0.17F;

		if (this->pole == rawrbox::Vector3f()) {
			if (!bent) return false;

			const ozz::math::SimdFloat4 bend = upper - limb * ozz::math::SplatX(ozz::math::Dot3(upper, limb));
			const ozz::math::SimdFloat4 polePos = ozz::math::NormalizeSafe3(bend, ozz::math::simd_float4::y_axis());

			this->pole = {ozz::math::GetX(polePos), ozz::math::GetY(polePos), ozz::math::GetZ(polePos)};
		}

		if (this->midAxis == rawrbox::Vector3f()) {
			ozz::math::SimdFloat4 axis = bendAxis;

			if (!bent) {
				axis = ozz::math::Cross3(limb, ozz::math::simd_float4::Load3PtrU(&this->pole.x));
				if (ozz::math::GetX(ozz::math::Length3Sqr(axis)) < 0.000001F) return false;
			}

			ozz::math::SimdInt4 invertible = ozz::math::simd_int4::zero();
			const auto inverse = ozz::math::Invert(blender.getRestModel(this->mid.value()), &invertible);
			if (!ozz::math::AreAllTrue1(invertible)) return false;

			const ozz::math::SimdFloat4 local = ozz::math::NormalizeSafe3(ozz::math::TransformVector(inverse, axis), ozz::math::simd_float4::z_axis());
			this->midAxis = {ozz::math::GetX(local), ozz::math::GetY(local), ozz::math::GetZ(local)};
		} else {
			this->midAxis = this->midAxis.normalized();
		}

		return true;
	}

	bool AnimationIK::solve(rawrbox::AnimationBlender& blender) {
		if (!this->joint.has_value() || !this->mid.has_value() || !this->start.has_value()) return false;

		ozz::math::SimdQuaternion startCorrection = ozz::math::SimdQuaternion::identity();
		ozz::math::SimdQuaternion midCorrection = ozz::math::SimdQuaternion::identity();

		ozz::animation::IKTwoBoneJob job;

		job.target = ozz::math::simd_float4::Load3PtrU(&this->target.x);
		job.pole_vector = ozz::math::simd_float4::Load3PtrU(&this->pole.x);
		job.mid_axis = ozz::math::simd_float4::Load3PtrU(&this->midAxis.x);

		job.soften = this->soften;
		job.twist_angle = this->twist;
		job.weight = this->weight;

		job.start_joint = &blender.getModel(this->start.value());
		job.mid_joint = &blender.getModel(this->mid.value());
		job.end_joint = &blender.getModel(this->joint.value());

		job.start_joint_correction = &startCorrection;
		job.mid_joint_correction = &midCorrection;
		job.reached = &this->reached;

		if (!job.Run()) return false;

		blender.rotateJoint(this->start.value(), startCorrection);
		blender.rotateJoint(this->mid.value(), midCorrection);

		return blender.updateModels(this->start.value());
	}
	// ------------
} // namespace rawrbox
