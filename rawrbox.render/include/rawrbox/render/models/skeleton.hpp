#pragma once

#include <rawrbox/math/matrix4x4.hpp>

#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/memory/unique_ptr.h>

#include <string>
#include <vector>

namespace rawrbox {
	class Skeleton {
	protected:
		ozz::unique_ptr<ozz::animation::Skeleton> _skeleton = nullptr;

		std::vector<rawrbox::Matrix4x4> _inverseBindMatrices = {};
		std::vector<int> _jointRemap = {};

	public:
		std::string name;

		Skeleton(std::string name, ozz::unique_ptr<ozz::animation::Skeleton> skeleton, std::vector<rawrbox::Matrix4x4> inverseBindMatrices, std::vector<int> jointRemap);
		Skeleton(const Skeleton&) = delete;
		Skeleton(Skeleton&&) = delete;
		Skeleton& operator=(const Skeleton&) = delete;
		Skeleton& operator=(Skeleton&&) = delete;
		virtual ~Skeleton() = default;

		// UTILS ---
		[[nodiscard]] virtual const ozz::animation::Skeleton& getSkeleton() const;

		[[nodiscard]] virtual size_t getNumJoints() const;
		[[nodiscard]] virtual size_t getNumSkinJoints() const;

		[[nodiscard]] virtual int getJointIndex(size_t index) const;
		[[nodiscard]] virtual const rawrbox::Matrix4x4& getInverseBindMatrix(size_t index) const;
		// ---------
	};
} // namespace rawrbox
