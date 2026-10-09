#include <rawrbox/render/models/animations/blender.hpp>
#include <rawrbox/utils/logger.hpp>

#include <ozz/animation/runtime/local_to_model_job.h>

#include <algorithm>

namespace rawrbox {
	AnimationBlender::AnimationBlender(rawrbox::Skeleton* skeleton) : _skeleton(skeleton) {
		if (this->_skeleton == nullptr) RAWRBOX_CRITICAL("Invalid skeleton");

		this->_locals.resize(this->_skeleton->getSkeleton().num_soa_joints());
		this->_models.resize(this->_skeleton->getNumJoints());
		this->_restModels.resize(this->_skeleton->getNumJoints());

		ozz::animation::LocalToModelJob restJob;

		restJob.skeleton = &this->_skeleton->getSkeleton();
		restJob.input = this->_skeleton->getSkeleton().joint_rest_poses();
		restJob.output = ozz::make_span(this->_restModels);

		if (!restJob.Run()) RAWRBOX_CRITICAL("Failed to compute rest pose of skeleton '{}'", this->_skeleton->name);
	}

	void AnimationBlender::reset() {
		this->_layers.clear();
		this->_addLayers.clear();
	}

	void AnimationBlender::addLayer(const ozz::vector<ozz::math::SoaTransform>& locals, float weight, const ozz::vector<ozz::math::SimdFloat4>& jointWeights, bool additive) {
		if (weight <= 0.F) return;

		if (locals.size() != this->_locals.size()) RAWRBOX_CRITICAL("Layer does not match skeleton '{}'! {} != {}", this->_skeleton->name, locals.size(), this->_locals.size());
		if (!jointWeights.empty() && jointWeights.size() != this->_locals.size()) RAWRBOX_CRITICAL("Joint weights do not match skeleton '{}'! {} != {}", this->_skeleton->name, jointWeights.size(), this->_locals.size());

		ozz::animation::BlendingJob::Layer layer;

		layer.weight = weight;
		layer.transform = ozz::make_span(locals);
		if (!jointWeights.empty()) layer.joint_weights = ozz::make_span(jointWeights);

		if (additive) {
			this->_addLayers.push_back(layer);
		} else {
			this->_layers.push_back(layer);
		}
	}

	// IK ---
	void AnimationBlender::removeIK(const std::string& id) {
		const int index = ozz::animation::FindJoint(this->_skeleton->getSkeleton(), id.c_str());
		if (index < 0) return;

		this->_ik.erase(index);
	}

	void AnimationBlender::clearIK() {
		this->_ik.clear();
	}

	const rawrbox::AnimationIKBase* AnimationBlender::getIK(const std::string& id) const {
		const int index = ozz::animation::FindJoint(this->_skeleton->getSkeleton(), id.c_str());
		if (index < 0) return nullptr;

		const auto fnd = this->_ik.find(index);
		return fnd == this->_ik.end() ? nullptr : fnd->second.get();
	}

	void AnimationBlender::rotateJoint(uint32_t id, const ozz::math::SimdQuaternion& rotation) {
		if (id >= this->_models.size()) RAWRBOX_CRITICAL("Invalid joint {} on skeleton '{}'", id, this->_skeleton->name);
		const int lane = static_cast<int>(id & 3U);

		ozz::math::SoaQuaternion delta = ozz::math::SoaQuaternion::identity();

		delta.x = ozz::math::SetI(delta.x, ozz::math::SplatX(rotation.xyzw), lane);
		delta.y = ozz::math::SetI(delta.y, ozz::math::SplatY(rotation.xyzw), lane);
		delta.z = ozz::math::SetI(delta.z, ozz::math::SplatZ(rotation.xyzw), lane);
		delta.w = ozz::math::SetI(delta.w, ozz::math::SplatW(rotation.xyzw), lane);

		ozz::math::SoaTransform& soa = this->_locals[id / 4];
		soa.rotation = soa.rotation * delta;
	}

	bool AnimationBlender::updateModels(uint32_t from) {
		ozz::animation::LocalToModelJob job;

		job.skeleton = &this->_skeleton->getSkeleton();
		job.input = ozz::make_span(this->_locals);
		job.output = ozz::make_span(this->_models);
		job.from = static_cast<int>(from);

		return job.Run();
	}
	// ------

	const ozz::vector<ozz::math::Float4x4>& AnimationBlender::blend() {
		const auto& skeleton = this->_skeleton->getSkeleton();

		ozz::animation::LocalToModelJob modelJob;
		modelJob.skeleton = &skeleton;
		modelJob.output = ozz::make_span(this->_models);

		if (this->_ik.empty() && this->_addLayers.empty() && this->_layers.size() == 1 && this->_layers[0].weight >= 1.F && this->_layers[0].joint_weights.empty()) {
			modelJob.input = this->_layers[0].transform;
		} else {
			ozz::animation::BlendingJob blendJob;
			blendJob.threshold = this->_threshold;
			blendJob.layers = ozz::make_span(this->_layers);
			blendJob.additive_layers = ozz::make_span(this->_addLayers);
			blendJob.rest_pose = skeleton.joint_rest_poses();
			blendJob.output = ozz::make_span(this->_locals);

			if (!blendJob.Run()) RAWRBOX_CRITICAL("Failed to blend animations on '{}'", this->_skeleton->name);
			modelJob.input = ozz::make_span(this->_locals);
		}

		if (!modelJob.Run()) RAWRBOX_CRITICAL("Failed to convert animations on '{}'", this->_skeleton->name);

		for (auto& ik : this->_ik) {
			if (!ik.second->solve(*this))
				RAWRBOX_CRITICAL("Failed to solve IK on '{}', joint '{}'", this->_skeleton->name, ik.first);
		}

		return this->_models;
	}

	// UTILS ---
	bool AnimationBlender::empty() const { return this->_layers.empty() && this->_addLayers.empty(); }
	bool AnimationBlender::hasIK() const { return !this->_ik.empty(); }
	rawrbox::Skeleton* AnimationBlender::getSkeleton() const { return this->_skeleton; }

	const ozz::math::Float4x4& AnimationBlender::getModel(uint32_t joint) const {
		if (joint >= this->_models.size()) RAWRBOX_CRITICAL("Invalid joint '{}' on '{}'", joint, this->_skeleton->name);
		return this->_models[joint];
	}

	const ozz::math::Float4x4& AnimationBlender::getRestModel(uint32_t joint) const {
		if (joint >= this->_restModels.size()) RAWRBOX_CRITICAL("Invalid joint '{}' on '{}'", joint, this->_skeleton->name);
		return this->_restModels[joint];
	}

	float AnimationBlender::getThreshold() const { return this->_threshold; }
	void AnimationBlender::setThreshold(float threshold) {
		this->_threshold = std::max(threshold, 0.001F);
	}
	// ---------
} // namespace rawrbox
