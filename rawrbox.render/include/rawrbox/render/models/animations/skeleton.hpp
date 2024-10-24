#pragma once

#include <rawrbox/render/models/animations/base.hpp>

#include <ozz/animation/runtime/skeleton.h>

#include <stdexcept>

namespace rawrbox {
	class AnimationSkeletonSampler : public rawrbox::AnimationSampler {
	private:
	public:
		AnimationSkeletonSampler(size_t index, ozz::animation::Animation* anim, std::function<void(const std::string&)> onComplete = nullptr) : rawrbox::AnimationSampler(index, anim, onComplete) {}

		// UTILS ----
		virtual ozz::vector<ozz::math::Float4x4> getOutput(const ozz::animation::Skeleton* skeleton) {
			if (skeleton == nullptr) throw std::runtime_error("Invalid skeleton");

			ozz::animation::SamplingJob job;
			job.context = &this->_context;
			job.animation = this->_animation;
			job.ratio = this->_currentTime;
			job.output = ozz::make_span(this->_output);

			if (!job.Run()) throw std::runtime_error("Failed to run animation job");

			ozz::vector<ozz::math::Float4x4> localOutput(skeleton->num_joints());

			ozz::animation::LocalToModelJob localJob;
			localJob.input = ozz::make_span(this->_output);
			localJob.output = ozz::make_span(localOutput);
			localJob.skeleton = skeleton;

			if (!localJob.Run()) throw std::runtime_error("Failed to run local to model job");

			return localOutput;
		}
		// -------------
	};
} // namespace rawrbox
