#pragma once

#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector3.hpp>

#include <array>
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

		/// MATRIX
		static inline void mtxTranslate(std::array<float, 16>& mtx, rawrBox::Vector3f pos) {
			mtx[12] = pos.x;
			mtx[13] = pos.y;
			mtx[14] = pos.z;
		}

		static inline void mtxScale(std::array<float, 16>& mtx, rawrBox::Vector3f scale) {
			mtx[0] *= scale.x;
			mtx[5] *= scale.y;
			mtx[10] *= scale.z;
			mtx[15] *= 1.0f;
		}

		static inline void vec4MulMtx(float* _result, const float* _vec, const float* _mat) {
			_result[0] = _vec[0] * _mat[0] + _vec[1] * _mat[4] + _vec[2] * _mat[8] + _vec[3] * _mat[12];
			_result[1] = _vec[0] * _mat[1] + _vec[1] * _mat[5] + _vec[2] * _mat[9] + _vec[3] * _mat[13];
			_result[2] = _vec[0] * _mat[2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] + _vec[3] * _mat[14];
			_result[3] = _vec[0] * _mat[3] + _vec[1] * _mat[7] + _vec[2] * _mat[11] + _vec[3] * _mat[15];
		}

		static inline std::array<float, 16> mtxMul(const std::array<float, 16>& mtxB, const std::array<float, 16>& mtx) {
			std::array<float, 16> _result = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			vec4MulMtx(&_result[0], &mtx[0], mtxB.data());
			vec4MulMtx(&_result[4], &mtx[4], mtxB.data());
			vec4MulMtx(&_result[8], &mtx[8], mtxB.data());
			vec4MulMtx(&_result[12], &mtx[12], mtxB.data());

			return _result;
		}

		static inline std::array<float, 16> mtxQuaternion(float w, float x, float y, float z) {
			std::array<float, 16> mtx = {};

			const float x2 = x + x;
			const float y2 = y + y;
			const float z2 = z + z;
			const float x2x = x2 * x;
			const float x2y = x2 * y;
			const float x2z = x2 * z;
			const float x2w = x2 * w;
			const float y2y = y2 * y;
			const float y2z = y2 * z;
			const float y2w = y2 * w;
			const float z2z = z2 * z;
			const float z2w = z2 * w;

			mtx[0] = 1.0f - (y2y + z2z);
			mtx[4] = x2y - z2w;
			mtx[8] = x2z + y2w;
			mtx[3] = 0.0f;

			mtx[1] = x2y + z2w;
			mtx[5] = 1.0f - (x2x + z2z);
			mtx[9] = y2z - x2w;
			mtx[7] = 0.0f;

			mtx[2] = x2z - y2w;
			mtx[6] = y2z + x2w;
			mtx[10] = 1.0f - (x2x + y2y);
			mtx[11] = 0.0f;

			mtx[12] = 0.0f;
			mtx[13] = 0.0f;
			mtx[14] = 0.0f;
			mtx[15] = 1.0f;

			return mtx;
		}
	};
} // namespace rawrBox
