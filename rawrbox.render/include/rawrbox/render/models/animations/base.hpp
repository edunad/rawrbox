#pragma once

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>

#include <functional>
#include <string>

namespace rawrbox {
	class AnimationSampler {
	protected:
		size_t _index = 0;
		ozz::animation::Animation* _animation = nullptr;
		std::function<void(const std::string&)> _onComplete = nullptr;

		float _currentTime = 0.F;
		float _playbackSpeed = 1.F;
		bool _loop = false;

		ozz::animation::SamplingJob::Context _context;
		ozz::vector<ozz::math::SoaTransform> _output;

	public:
		AnimationSampler(size_t index, ozz::animation::Animation* anim, std::function<void(const std::string&)> onComplete = nullptr) : _animation(anim), _index(index), _onComplete(onComplete) {
			this->_context.Resize(anim->num_tracks());
			this->_output.resize(anim->num_soa_tracks());
		}

		virtual ~AnimationSampler() {
			if (this->_onComplete != nullptr) this->_onComplete(this->_animation->name());

			this->_animation = nullptr;
			this->_context.Invalidate();
			this->_output.clear();
		}

		virtual bool tick(float deltaTime) {
			float timeToAdd = deltaTime * this->getSpeed();
			float newTime = this->getTime() + (timeToAdd / this->getDuration());

			// Clamp newTime to the range [0, 1]
			newTime = std::max(0.0f, std::min(1.0f, newTime));

			bool animationEnded = this->getSpeed() >= 0.F ? newTime >= 1.F : newTime <= 0.F;
			if (animationEnded) {
				if (!this->getLoop()) return true;
				newTime = this->getSpeed() >= 0.F ? 0.F : 1.F;
			}

			this->setTime(newTime);
			return false;
		}

		// UTILS ----
		virtual float getDuration() const { return this->_animation->duration(); }
		virtual size_t getIndex() const { return this->_index; }

		virtual ozz::animation::AnimationType getType() const { return this->_animation->type; }

		virtual float getTime() const { return this->_currentTime; }
		virtual float setTime(float time) { return this->_currentTime = time; }

		virtual bool getLoop() const { return this->_loop; }
		virtual void setLoop(bool loop) { this->_loop = loop; }

		virtual float getSpeed() const { return this->_playbackSpeed; }
		virtual void setSpeed(float speed) { this->_playbackSpeed = speed; }

		virtual ozz::animation::Animation* getAnimation() const { return this->_animation; }
		// -------------
	};
} // namespace rawrbox
