#pragma once
#include <rawrbox/render/model/base.hpp>

#include <assimp/quaternion.h>
#include <assimp/vector3.h>

#include <unordered_map>

namespace rawrBox {

	template <typename T>
	struct AnimKey {
		float time;
		T value;
	};

	struct AnimationFrame {
		std::string nodeName;

		std::vector<AnimKey<aiVector3D>> position;
		std::vector<AnimKey<aiVector3D>> scale;
		std::vector<AnimKey<aiQuaternion>> rotation;
	};

	struct Animation {
		float ticksPerSecond = 0;
		float duration = 0;
		std::vector<AnimationFrame> frames;
	};

	struct PlayingAnimationData {
		std::string name;

		bool loop = false;
		float speed = 1;
		float time = 0;

		Animation* data = nullptr;
		PlayingAnimationData() = default;
		PlayingAnimationData(const std::string& _name, bool _loop, float _speed, float _time, Animation* _data) : name(_name), loop(_loop), speed(_speed), time(_time), data(_data){};
	};

	class Model : public rawrBox::ModelBase {
	protected:
		std::unordered_map<std::string, Animation> _animations = {};
		std::unordered_map<uint16_t, std::array<float, 16>> _boneCalcs = {};

		std::unique_ptr<rawrBox::PlayingAnimationData> _currentAnimation = nullptr;

		void updateBones(std::shared_ptr<rawrBox::Mesh> mesh);
		void readAnim(Skeleton* skeleton, Bone& node, const std::array<float, 16>& transform);

		void preDraw();
		void postDraw();

	public:
		using ModelBase::ModelBase;

		// Animations ----
		bool playAnimation(const std::string& name, bool loop = true, float speed = 1.f);
		bool stopAnimation();
		// -----

		void draw(const rawrBox::Vector3f& camPos) override;
	};
} // namespace rawrBox
