#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/pi.hpp>

#include <cmath>
#include <vector>

namespace rawrbox {
	class MathUtils {
	public:
		template <typename T = int>
			requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
		static inline T repeat(T val, T min, T max) {
			if (val < 0 && max > 0) val = max - std::abs(val - min);
			return ((val - min) % max) + min;
		}

		template <typename T = int>
			requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
		static inline T pingPong(T val, T max) {
			return static_cast<T>(val - (std::floor(val / max) * max));
		}

		static float toRad(float val);
		static float toDeg(float val);

		static float round(float val, int precision = 2);

		template <typename T = float>
			requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
		static T divideRound(T nominator, T denominator) {
			return (nominator + denominator - static_cast<T>(1)) / denominator;
		}

		static float lerp(float a, float b, float lerpFactor);

		static float sample(const std::vector<float>& samples, float t);

		// https://gist.github.com/itsmrpeck/be41d72e9d4c72d2236de687f6f53974
		static float angleLerp(float a, float b, float lerpFactor);

		static float angleRadLerp(float a, float b, float lerpFactor);

		static int nextPow2(int pow);

		// In DEG
		static float angleDistance(float A, float B);

		// VEC
		static rawrbox::Vector3f applyRotation(const rawrbox::Vector3f& vert, const rawrbox::Vector4f& ang);

		static Vector2f toRad(const Vector2f& vec);
		static Vector2f toDeg(const Vector2f& vec);

		static Vector3f toRad(const Vector3f& vec);
		static Vector3f toDeg(const Vector3f& vec);
	};
} // namespace rawrbox
