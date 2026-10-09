#pragma once

#include <rawrbox/render/models/animations/ik.hpp>
#include <rawrbox/render/models/skeleton.hpp>

#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/skeleton_utils.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/simd_quaternion.h>
#include <ozz/base/maths/soa_transform.h>

#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace rawrbox {
	class AnimationBlender {
	protected:
		rawrbox::Skeleton* _skeleton = nullptr;

		std::vector<ozz::animation::BlendingJob::Layer> _layers = {};
		std::vector<ozz::animation::BlendingJob::Layer> _addLayers = {};

		std::map<int, std::unique_ptr<rawrbox::AnimationIKBase>> _ik = {};

		ozz::vector<ozz::math::SoaTransform> _locals = {};
		ozz::vector<ozz::math::Float4x4> _models = {};
		ozz::vector<ozz::math::Float4x4> _restModels = {};

		float _threshold = 0.1F;

	public:
		explicit AnimationBlender(rawrbox::Skeleton* skeleton);
		AnimationBlender(const AnimationBlender&) = delete;
		AnimationBlender(AnimationBlender&&) = delete;
		AnimationBlender& operator=(const AnimationBlender&) = delete;
		AnimationBlender& operator=(AnimationBlender&&) = delete;
		virtual ~AnimationBlender() = default;

		virtual void reset();
		virtual void addLayer(const ozz::vector<ozz::math::SoaTransform>& locals, float weight, const ozz::vector<ozz::math::SimdFloat4>& jointWeights = {}, bool additive = false);

		// IK ---
		template <typename T>
			requires(std::derived_from<T, rawrbox::AnimationIKBase>)
		bool addIK(const std::string& id, const T& ik) {
			const int index = ozz::animation::FindJoint(this->_skeleton->getSkeleton(), id.c_str());
			if (index < 0) return false;

			const auto fnd = this->_ik.find(index);
			if (fnd != this->_ik.end() && typeid(*fnd->second) == typeid(T)) {
				T& current = *static_cast<T*>(fnd->second.get());
				T copy = ik;
				if (!copy.setup(*this, static_cast<uint32_t>(index))) return false;

				current = copy;
				return true;
			}

			auto copy = std::make_unique<T>(ik);
			if (!copy->setup(*this, static_cast<uint32_t>(index))) return false;

			this->_ik[index] = std::move(copy);
			return true;
		}

		virtual void removeIK(const std::string& id);
		virtual void clearIK();
		[[nodiscard]] virtual const rawrbox::AnimationIKBase* getIK(const std::string& id) const;

		virtual void rotateJoint(uint32_t id, const ozz::math::SimdQuaternion& rotation);
		virtual bool updateModels(uint32_t from);
		// ------

		[[nodiscard]] virtual const ozz::vector<ozz::math::Float4x4>& blend();

		// UTILS ---
		[[nodiscard]] virtual bool empty() const;
		[[nodiscard]] virtual bool hasIK() const;
		[[nodiscard]] virtual rawrbox::Skeleton* getSkeleton() const;

		[[nodiscard]] virtual const ozz::math::Float4x4& getModel(uint32_t joint) const;
		[[nodiscard]] virtual const ozz::math::Float4x4& getRestModel(uint32_t joint) const;

		[[nodiscard]] virtual float getThreshold() const;
		virtual void setThreshold(float threshold);
		// ---------
	};
} // namespace rawrbox
