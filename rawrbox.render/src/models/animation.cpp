#include <rawrbox/render/models/animation.hpp>
#include <rawrbox/render/models/skeleton.hpp>
#include <rawrbox/utils/logger.hpp>

#include <algorithm>

namespace rawrbox {
	Animation::Animation(std::string name) : name(std::move(name)) {}

	// PARTS ---
	void Animation::addPart(rawrbox::AnimationPart part) {
		if (part.animation == nullptr) RAWRBOX_CRITICAL("Animation '{}' has no ozz animation", this->name);

		const auto tracks = static_cast<size_t>(part.animation->num_tracks());
		switch (part.type) {
			case rawrbox::AnimationType::SKELETON:
				if (part.skeleton == nullptr) RAWRBOX_CRITICAL("Animation '{}' is missing skeleton", this->name);
				if (tracks != part.skeleton->getNumJoints()) RAWRBOX_CRITICAL("Animation '{}' has {} tracks but skeleton '{}' has {} joints", this->name, tracks, part.skeleton->name, part.skeleton->getNumJoints());
				break;
			case rawrbox::AnimationType::VERTEX:
				if (tracks != part.meshes.size()) RAWRBOX_CRITICAL("Animation '{}' has {} tracks but targets {} meshes", this->name, tracks, part.meshes.size());
				break;
		}

		this->duration = std::max(this->duration, part.animation->duration());
		this->_parts.push_back(std::move(part));
	}

	const std::vector<rawrbox::AnimationPart>& Animation::getParts() const {
		return this->_parts;
	}
	// ---------

	bool Animation::empty() const {
		return this->_parts.empty();
	}
} // namespace rawrbox
