#include <rawrbox/render/models/animations/sampler.hpp>
#include <rawrbox/render/models/skeleton.hpp>
#include <rawrbox/utils/logger.hpp>

#include <ozz/animation/runtime/skeleton_utils.h>

#include <algorithm>
#include <cmath>

namespace rawrbox {
	AnimationSampler::AnimationSampler(size_t index, rawrbox::Animation* animation, std::function<void(const std::string&)> onComplete) : _index(index), _animation(animation), _onComplete(std::move(onComplete)) {
		if (this->_animation == nullptr) RAWRBOX_CRITICAL("Invalid animation");
		if (this->_animation->empty()) RAWRBOX_CRITICAL("Animation '{}' has no parts", this->_animation->name);

		for (const auto& part : this->_animation->getParts()) {
			auto state = std::make_unique<PartState>();
			state->context.Resize(part.animation->num_tracks());
			state->locals.resize(part.animation->num_soa_tracks());

			this->_parts.push_back(std::move(state));
		}
	}

	AnimationSampler::~AnimationSampler() {
		this->_onComplete = nullptr;
		this->_parts.clear();
		this->_animation = nullptr;
	}

	void AnimationSampler::complete() {
		if (this->_onComplete == nullptr) return;

		auto callback = std::move(this->_onComplete);
		this->_onComplete = nullptr;

		callback(this->_animation->name);
	}

	bool AnimationSampler::tick(float deltaTime) {
		const float duration = this->getDuration();
		if (duration <= 0.F) return true;

		const bool forward = this->_playbackSpeed >= 0.F;
		float newTime = this->_currentTime + (deltaTime * this->_playbackSpeed) / duration;

		const bool ended = forward ? newTime >= 1.F : newTime <= 0.F;
		if (!ended) {
			this->_currentTime = newTime;
			return false;
		}

		if (this->_loop) {
			newTime -= std::floor(newTime);
			this->_currentTime = std::clamp(newTime, 0.F, 1.F);
			return false;
		}

		this->_currentTime = forward ? 1.F : 0.F;
		return true;
	}

	void AnimationSampler::sample() {
		const auto& parts = this->_animation->getParts();

		for (size_t i = 0; i < parts.size(); i++) {
			const auto& part = parts[i];
			const auto& state = this->_parts[i];

			ozz::animation::SamplingJob job;
			job.animation = part.animation.get();
			job.context = &state->context;
			job.ratio = this->_currentTime;
			job.output = ozz::make_span(state->locals);

			if (!job.Run()) RAWRBOX_CRITICAL("Failed to sample animation '{}' (part {})", this->_animation->name, i);
		}
	}

	// OUTPUT ---
	const ozz::vector<ozz::math::SoaTransform>& AnimationSampler::getLocalOutput(size_t part) const {
		if (part >= this->_parts.size()) RAWRBOX_CRITICAL("Invalid animation part {}", part);
		return this->_parts[part]->locals;
	}

	const ozz::vector<ozz::math::SimdFloat4>& AnimationSampler::getJointWeights(size_t part) const {
		if (part >= this->_parts.size()) RAWRBOX_CRITICAL("Invalid animation part {}", part);
		return this->_parts[part]->jointWeights;
	}
	// ----------

	// UTILS ----
	float AnimationSampler::getDuration() const { return this->_animation->duration; }
	size_t AnimationSampler::getIndex() const { return this->_index; }

	float AnimationSampler::getTime() const { return this->_currentTime; }
	void AnimationSampler::setTime(float time) { this->_currentTime = std::clamp(time, 0.F, 1.F); }

	bool AnimationSampler::getLoop() const { return this->_loop; }
	void AnimationSampler::setLoop(bool loop) { this->_loop = loop; }

	float AnimationSampler::getSpeed() const { return this->_playbackSpeed; }
	void AnimationSampler::setSpeed(float speed) {
		if (speed < 0.F && this->_playbackSpeed >= 0.F && this->_currentTime <= 0.F) this->_currentTime = 1.F;
		if (speed >= 0.F && this->_playbackSpeed < 0.F && this->_currentTime >= 1.F) this->_currentTime = 0.F;

		this->_playbackSpeed = speed;
	}

	float AnimationSampler::getWeight() const { return this->_weight; }
	void AnimationSampler::setWeight(float weight) { this->_weight = std::max(weight, 0.F); }

	bool AnimationSampler::setJointWeight(const std::string& id, float weight, bool children) {
		bool found = false;

		const auto& parts = this->_animation->getParts();
		for (size_t i = 0; i < parts.size(); i++) {
			const auto& part = parts[i];
			if (part.type != rawrbox::AnimationType::SKELETON) continue;

			const auto& skeleton = part.skeleton->getSkeleton();
			const int index = ozz::animation::FindJoint(skeleton, id.c_str());
			if (index < 0) continue;

			auto& mask = this->_parts[i]->jointWeights;
			if (mask.empty()) mask.resize(skeleton.num_soa_joints(), ozz::math::simd_float4::one());

			const auto value = ozz::math::simd_float4::Load1(std::clamp(weight, 0.F, 1.F));
			const auto apply = [&mask, &value](int j, int /*parent*/) { mask[j / 4] = ozz::math::SetI(mask[j / 4], value, j % 4); };

			if (children) {
				ozz::animation::IterateJointsDF(skeleton, apply, index);
			} else {
				apply(index, 0);
			}

			found = true;
		}

		return found;
	}

	void AnimationSampler::clearJointWeights() {
		for (const auto& part : this->_parts) {
			part->jointWeights.clear();
		}
	}

	rawrbox::Animation* AnimationSampler::getAnimation() const { return this->_animation; }
	// -------------
} // namespace rawrbox
