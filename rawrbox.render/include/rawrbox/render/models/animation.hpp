#pragma once

#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_track.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/maths/soa_transform.h>

#include <stdexcept>

namespace rawrbox {
	class AnimationSampler {
	private:
		size_t _index = 0;
		ozz::animation::Animation* _animation = nullptr;

		ozz::animation::SamplingJob::Context _sample;
		ozz::vector<ozz::math::SoaTransform> _output;
		ozz::vector<ozz::math::Float4x4> _modelOutput;

		float _currentTime = 0.F;
		float _playbackSpeed = 1.F;
		bool _loop = false;

	public:
		AnimationSampler(size_t index, ozz::animation::Animation* anim) : _index(index), _animation(anim) {
			this->_sample.Resize(_animation->num_tracks());
			this->_output.resize(_animation->num_soa_tracks());
		}

		~AnimationSampler() {
			this->_animation = nullptr;
			this->_output.clear();
			this->_sample.Invalidate();
		}

		void sample(ozz::animation::Skeleton* skeleton = nullptr) {
			ozz::animation::SamplingJob job;
			job.context = &this->_sample;
			job.animation = this->_animation;
			job.ratio = this->_currentTime;
			job.output = ozz::make_span(this->_output);

			if (!job.Run()) throw std::runtime_error("Failed to run animation job");
			if (skeleton == nullptr) return;

			ozz::animation::LocalToModelJob localJob;
			localJob.input = ozz::make_span(this->_output);
			localJob.output = ozz::make_span(this->_modelOutput);
			localJob.skeleton = skeleton;

			if (!localJob.Run()) throw std::runtime_error("Failed to run animation job");
		}

		// UTILS ----
		float getDuration() const { return this->_animation->duration(); }
		size_t getIndex() const { return this->_index; }

		float getTime() const { return this->_currentTime; }
		float setTime(float time) { return this->_currentTime = time; }

		bool getLoop() const { return this->_loop; }
		void setLoop(bool loop) { this->_loop = loop; }

		float getSpeed() const { return this->_playbackSpeed; }
		void setSpeed(float speed) { this->_playbackSpeed = speed; }

		ozz::animation::Animation* getAnimation() const { return this->_animation; }
		const ozz::vector<ozz::math::SoaTransform>& getOutput() const { return this->_output; }
		const ozz::vector<ozz::math::Float4x4>& getModelOutput() const { return this->_modelOutput; }
		// -------------
	};
} // namespace rawrbox
