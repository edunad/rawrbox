#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/pi.hpp>

#include <array>
#include <cmath>
#include <vector>

namespace rawrbox {
	class MathUtils {
	public:
		template <typename T = int>
		static inline T repeat(T val, T min, T max) {
			if (val < 0 && max > 0) val = max - std::abs(val - min);
			return ((val - min) % max) + min;
		}

		template <typename T = int>
		static inline T pingPong(T val, T max) {
			return static_cast<T>(val - (std::floor(val / max) * max));
		}

		static inline float toRad(float val) {
			return val * rawrbox::pi<float> / 180.F;
		}

		static inline float toDeg(float val) {
			return val * 180.F / rawrbox::pi<float>;
		}

		static inline float lerp(float a, float b, float lerpFactor) {
			return ((1.F - lerpFactor) * a) + (lerpFactor * b);
		}

		static inline float sample(const std::vector<float>& samples, float t) {
			int count = samples.size();
			if (count == 0) return 0;
			if (count == 1) return samples[0];

			float f = t * (count - 1);
			int idLower = static_cast<int>(std::floor(f));
			int idUpper = static_cast<int>(std::floor(f + 1));

			if (idUpper >= count) return samples[count - 1];
			if (idLower < 0) return samples[0];

			return std::lerp(samples[idLower], samples[idUpper], f - idLower);
		}

		// https://gist.github.com/itsmrpeck/be41d72e9d4c72d2236de687f6f53974
		static inline float angleLerp(float a, float b, float lerpFactor) {
			float result = NAN;
			float diff = b - a;

			if (diff < -180.F) {
				// lerp upwards past 360
				b += 360.F;
				result = lerp(a, b, lerpFactor);
				if (result >= 360.F) result -= 360.F;
			} else if (diff > 180.F) {
				// lerp downwards past 0
				b -= 360.F;
				result = lerp(a, b, lerpFactor);
				if (result < 0.F) result += 360.F;
			} else {
				// straight lerp
				result = lerp(a, b, lerpFactor);
			}

			return result;
		}

		static inline float angleRadLerp(float a, float b, float lerpFactor) {
			if (a == b) return b;

			float PI = rawrbox::pi<float>;
			float PI_TIMES_TWO = PI * 2;

			float result = 0.F;
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
				if (result < 0.F) result += PI_TIMES_TWO;
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

		// In DEG
		static inline float angleDistance(float A, float B) {
			if (A >= 180) A = -180.F + std::fmod(A + 180, 360.F);
			if (A <= -180) A = 180.F + std::fmod(A + 180, -360.F);
			if (B >= 180) B = -180.F + std::fmod(B + 180, 360.F);
			if (B <= -180) B = 180.F + std::fmod(B + 180, -360.F);

			auto diff = std::abs(A - B);
			return std::min(diff, 360.F - diff);
		}

		// VEC
		static inline rawrbox::Vector3f applyRotation(const rawrbox::Vector3f& vert, const rawrbox::Vector4f& ang) {
			rawrbox::Matrix4x4 translationMatrix = {};
			translationMatrix.translate({-ang.x, -ang.y, ang.z});

			rawrbox::Matrix4x4 rotationMatrix = {};
			rotationMatrix.rotateXYZ(ang.xyz());

			rawrbox::Matrix4x4 reverseTranslationMatrix = {};
			reverseTranslationMatrix.translate({ang.x, ang.y, ang.z});

			rawrbox::Matrix4x4 mul = translationMatrix * rotationMatrix * reverseTranslationMatrix;

			rawrbox::Vector4f v = {vert.x, vert.y, vert.z, -1.0F};
			auto res = mul.mulVec(v);

			rawrbox::Vector3f out;
			out.x = res.x;
			out.y = res.y;
			out.z = res.z;

			return out;
		}
	};
} // namespace rawrbox
