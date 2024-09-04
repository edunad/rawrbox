#pragma once

/*
#include <rawrbox/math/easing.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <functional>
#include <utility>
#include <vector>
*/

#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_track.h>

namespace rawrbox {
	/*template <typename T>
	using AnimKey = std::pair<float, T>;

	struct AnimationFrame {
		size_t nodeIndex; // Mesh node being affected

		std::vector<rawrbox::AnimKey<rawrbox::Vector3f>> position;
		std::vector<rawrbox::AnimKey<rawrbox::Vector3f>> scale;
		std::vector<rawrbox::AnimKey<rawrbox::Vector4f>> rotation;

		rawrbox::Easing stateStart = rawrbox::Easing::LINEAR;
		rawrbox::Easing stateEnd = rawrbox::Easing::LINEAR;
	};

	struct Animation {
		float duration = 0;
		std::vector<rawrbox::AnimationFrame> frames = {};
	};

	struct PlayingAnimationData {
		std::string name;

		bool loop = false;
		float speed = 1;
		float time = 0;

		rawrbox::Animation* data = nullptr;
		std::function<void()> onComplete = nullptr;

		PlayingAnimationData() = default;
		PlayingAnimationData(std::string _name, bool _loop, float _speed, rawrbox::Animation& _data, std::function<void()> complete = nullptr) : name(std::move(_name)), loop(_loop), speed(_speed), time(_speed <= 0.F ? _data.duration : 0.F), data(&_data), onComplete(std::move(complete)) {};
	};*/
} // namespace rawrbox
