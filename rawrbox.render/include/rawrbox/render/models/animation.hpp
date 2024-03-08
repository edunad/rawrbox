#pragma once

#include <rawrbox/math/easing.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <string>
#include <vector>

namespace rawrbox {

	template <typename T>
	using AnimKey = std::pair<float, T>;

	struct AnimationFrame {
		std::string nodeName;

		std::vector<AnimKey<rawrbox::Vector3f>> position;
		std::vector<AnimKey<rawrbox::Vector3f>> scale;
		std::vector<AnimKey<rawrbox::Vector4f>> rotation;

		rawrbox::Easing stateStart = rawrbox::Easing::LINEAR;
		rawrbox::Easing stateEnd = rawrbox::Easing::LINEAR;
	};

	struct Animation {
		float ticksPerSecond = 0;
		float duration = 0;

		std::vector<AnimationFrame> frames = {};
	};

	struct PlayingAnimationData {
		std::string name;

		bool loop = false;
		float speed = 1;
		float time = 0;

		Animation* data = nullptr;
		PlayingAnimationData() = default;
		PlayingAnimationData(std::string _name, bool _loop, float _speed, float _time, Animation* _data) : name(std::move(_name)), loop(_loop), speed(_speed), time(_time), data(_data){};
	};
} // namespace rawrbox
