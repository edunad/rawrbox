#pragma once

#include <rawrbox/render/models/animation.hpp>

#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace rawrbox {
	class AnimationSampler {
	protected:
		struct PartState {
			ozz::animation::SamplingJob::Context context = {};
			ozz::vector<ozz::math::SoaTransform> locals = {};
			ozz::vector<ozz::math::Float4x4> models = {};

			bool dirty = true;
		};

		size_t _index = 0;
		
		rawrbox::Animation* _animation = nullptr;
		std::function<void(const std::string&)> _onComplete = nullptr;

		std::vector<std::unique_ptr<PartState>> _parts = {};

		float _currentTime = 0.F; // 0 -> 1
		float _playbackSpeed = 1.F;
		bool _loop = false;

	public:
		AnimationSampler(size_t index, rawrbox::Animation* animation, std::function<void(const std::string&)> onComplete = nullptr);
		AnimationSampler(const AnimationSampler&) = delete;
		AnimationSampler(AnimationSampler&&) = delete;
		AnimationSampler& operator=(const AnimationSampler&) = delete;
		AnimationSampler& operator=(AnimationSampler&&) = delete;
		virtual ~AnimationSampler();

		virtual bool tick(float deltaTime);

		virtual void sample();
		virtual void complete();

		// SAMPLE --
		[[nodiscard]] virtual const ozz::vector<ozz::math::SoaTransform>& getLocalOutput(size_t part) const;
		[[nodiscard]] virtual const ozz::vector<ozz::math::Float4x4>& getModelOutput(size_t part);
		// ----------

		// UTILS ----
		[[nodiscard]] virtual float getDuration() const;
		[[nodiscard]] virtual size_t getIndex() const;

		[[nodiscard]] virtual float getTime() const;
		virtual void setTime(float time);

		[[nodiscard]] virtual bool getLoop() const;
		virtual void setLoop(bool loop);

		[[nodiscard]] virtual float getSpeed() const;
		virtual void setSpeed(float speed);

		[[nodiscard]] virtual rawrbox::Animation* getAnimation() const;
		// -------------
	};
} // namespace rawrbox
