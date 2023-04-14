#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

// From https://github.com/bkaradzic/bgfx/blob/master/examples/33-pom/pom.cpp#L56
namespace rawrBox {
	class PackUtils {
	public:
		static uint32_t packUint32(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w) {
			union {
				uint32_t ui32;
				uint8_t arr[4];
			} un;

			un.arr[0] = _x;
			un.arr[1] = _y;
			un.arr[2] = _z;
			un.arr[3] = _w;

			return un.ui32;
		}

		static uint32_t packF4u(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f) {
			const uint8_t xx = uint8_t(_x * 127.0f + 128.0f);
			const uint8_t yy = uint8_t(_y * 127.0f + 128.0f);
			const uint8_t zz = uint8_t(_z * 127.0f + 128.0f);
			const uint8_t ww = uint8_t(_w * 127.0f + 128.0f);

			return packUint32(xx, yy, zz, ww);
		}

		static void packRgba8(void* _dst, const float* _src) {
			uint8_t* dst = (uint8_t*)_dst;
			dst[0] = uint8_t(toUnorm(_src[0], 255.0f));
			dst[1] = uint8_t(toUnorm(_src[1], 255.0f));
			dst[2] = uint8_t(toUnorm(_src[2], 255.0f));
			dst[3] = uint8_t(toUnorm(_src[3], 255.0f));
		}

		static uint32_t toUnorm(float _value, float _scale) {
			return uint32_t(std::round(std::clamp(_value, 0.0f, 1.0f) * _scale));
		}

		static float fromUnorm(uint32_t _value, float _scale) {
			return float(_value) / _scale;
		}

		static int32_t toSnorm(float _value, float _scale) {
			return int32_t(std::round(
			    std::clamp(_value, -1.0f, 1.0f) * _scale));
		}

		static float fromSnorm(int32_t _value, float _scale) {
			return std::max(-1.0f, float(_value) / _scale);
		}

		static uint32_t packNormal(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f) {
			const float src[] =
			    {
				_x * 0.5f + 0.5f,
				_y * 0.5f + 0.5f,
				_z * 0.5f + 0.5f,
				_w * 0.5f + 0.5f,
			    };
			uint32_t dst;
			PackUtils::packRgba8(&dst, src);
			return dst;
		}
	};
} // namespace rawrBox
