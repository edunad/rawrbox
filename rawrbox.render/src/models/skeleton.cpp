#include <rawrbox/render/models/skeleton.hpp>
#include <rawrbox/render/render_config.hpp>
#include <rawrbox/utils/logger.hpp>

namespace rawrbox {
	Skeleton::Skeleton(std::string name, ozz::unique_ptr<ozz::animation::Skeleton> skeleton, std::vector<rawrbox::Matrix4x4> inverseBindMatrices, std::vector<int> jointRemap) : _skeleton(std::move(skeleton)), _inverseBindMatrices(std::move(inverseBindMatrices)), _jointRemap(std::move(jointRemap)), name(std::move(name)) {
		if (this->_skeleton == nullptr) RAWRBOX_CRITICAL("Invalid ozz skeleton for skeleton '{}'", this->name);

		if (this->_jointRemap.empty()) RAWRBOX_CRITICAL("Skeleton '{}' has no joints", this->name);
		if (this->_jointRemap.size() != this->_inverseBindMatrices.size()) RAWRBOX_CRITICAL("Skeleton '{}' {} joints do not match {} inverse bind matrices", this->name, this->_jointRemap.size(), this->_inverseBindMatrices.size());
		if (this->_jointRemap.size() > RB_RENDER_MAX_BONES_PER_MODEL) RAWRBOX_CRITICAL("Skeleton '{}' has {} skin joints but max is {}", this->name, this->_jointRemap.size(), RB_RENDER_MAX_BONES_PER_MODEL);

		const int numJoints = this->_skeleton->num_joints();
		for (size_t i = 0; i < this->_jointRemap.size(); i++) {
			const int joint = this->_jointRemap[i];
			if (joint < 0 || joint >= numJoints) RAWRBOX_CRITICAL("Invalid skeleton '{}' -> joint {}!", this->name, joint);
		}
	}

	// UTILS ---
	const ozz::animation::Skeleton& Skeleton::getSkeleton() const {
		return *this->_skeleton;
	}

	size_t Skeleton::getNumJoints() const {
		return static_cast<size_t>(this->_skeleton->num_joints());
	}

	size_t Skeleton::getNumSkinJoints() const {
		return this->_jointRemap.size();
	}

	int Skeleton::getJointIndex(size_t index) const {
		if (index >= this->_jointRemap.size()) RAWRBOX_CRITICAL("Invalid joint index {}, skeleton '{}'", index, this->name);
		return this->_jointRemap[index];
	}

	const rawrbox::Matrix4x4& Skeleton::getInverseBindMatrix(size_t index) const {
		if (index >= this->_inverseBindMatrices.size()) RAWRBOX_CRITICAL("Mssing joint index {}, skeleton '{}'", index, this->name);
		return this->_inverseBindMatrices[index];
	}
	// ---------
} // namespace rawrbox
