#pragma once

#include <rawrbox/math/pi.hpp>

#include <cmath>

namespace rawrBox {
	class MathUtils {
	public:
		template <typename T = int>
		static inline T clamp(T val, T min, T max) {
			return std::min(std::max(val, min), max);
		}

		template <typename T = int>
		static inline T repeat(T val, T min, T max) {
			return ((val - min) % max) + min;
		}

		static inline float toRad(float val) {
			return val * rawrBox::pi<float> / 180.f;
		}

		static inline float toDeg(float val) {
			return val * 180.f / rawrBox::pi<float>;
		}

		static inline float lerp(float a, float b, float lerpFactor) {
			return ((1.f - lerpFactor) * a) + (lerpFactor * b);
		}

		// https://gist.github.com/itsmrpeck/be41d72e9d4c72d2236de687f6f53974
		static inline float angleLerp(float a, float b, float lerpFactor) {
			float result = NAN;
			float diff = b - a;

			if (diff < -180.f) {
				// lerp upwards past 360
				b += 360.f;
				result = lerp(a, b, lerpFactor);
				if (result >= 360.f) result -= 360.f;
			} else if (diff > 180.f) {
				// lerp downwards past 0
				b -= 360.f;
				result = lerp(a, b, lerpFactor);
				if (result < 0.f) result += 360.f;
			} else {
				// straight lerp
				result = lerp(a, b, lerpFactor);
			}

			return result;
		}

		static inline float angleRadLerp(float a, float b, float lerpFactor) {
			if (a == b) return b;

			float PI = rawrBox::pi<float>;
			float PI_TIMES_TWO = PI * 2;

			float result = 0.f;
			float diff = b - a;

			if (diff < -PI) {
				// lerp upwards past PI_TIMES_TWO
				b += PI_TIMES_TWO;
				result = lerp(a, b, lerpFactor);
				if (result >= PI_TIMES_TWO) result -= PI_TIMES_TWO;

			} else if (diff > PI) {
				// lerp downwards past 0
				b -= PI_TIMES_TWO;
				result = lerp(a, b, lerpFactor);
				if (result < 0.f) result += PI_TIMES_TWO;
			} else {
				// straight lerp
				result = lerp(a, b, lerpFactor);
			}

			return result;
		}

		static inline int nextPow2(int pow) {
			if (pow != 0 && (pow & (pow - 1)) != 0) {
				pow--;
				pow |= pow >> 1;
				pow |= pow >> 2;
				pow |= pow >> 4;
				pow |= pow >> 8;
				pow |= pow >> 16;
				pow++;
			}

			return pow;
		}
	};
} // namespace rawrBox
