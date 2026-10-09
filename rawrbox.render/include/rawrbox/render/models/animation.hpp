#pragma once

#include <ozz/animation/runtime/animation.h>
#include <ozz/base/memory/unique_ptr.h>

#include <memory>
#include <string>
#include <vector>

namespace rawrbox {
	class Skeleton;

	enum class AnimationType : uint8_t {
		VERTEX = 0,
		SKELETON = 1
	};

	struct AnimationPart {
		rawrbox::AnimationType type = rawrbox::AnimationType::VERTEX;
		bool additive = false;

		ozz::unique_ptr<ozz::animation::Animation> animation = nullptr;

		rawrbox::Skeleton* skeleton = nullptr;
		std::vector<uint32_t> meshes = {};
	};

	class Animation {
	protected:
		std::vector<rawrbox::AnimationPart> _parts = {};

	public:
		std::string name;
		float duration = 0.F;

		explicit Animation(std::string name);
		Animation(const Animation&) = delete;
		Animation(Animation&&) = delete;
		Animation& operator=(const Animation&) = delete;
		Animation& operator=(Animation&&) = delete;
		virtual ~Animation() = default;

		// PARTS ---
		virtual void addPart(rawrbox::AnimationPart part);
		[[nodiscard]] virtual const std::vector<rawrbox::AnimationPart>& getParts() const;
		// ---------
		
		[[nodiscard]] virtual bool empty() const;
	};
} // namespace rawrbox
