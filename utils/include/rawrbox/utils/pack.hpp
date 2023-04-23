#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// From https://github.com/bkaradzic/bgfx/blob/master/examples/33-pom/pom.cpp#L56
namespace rawrBox {
	class PackUtils {
	public:
		static uint32_t packUint32(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w) {
			return bit_cast<uint32_t>(std::array<uint8_t, 4>{_x, _y, _z, _w});
		}

		static uint32_t packF4u(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f) {
			const auto xx = uint8_t(_x * 127.0f + 128.0f);
			const auto yy = uint8_t(_y * 127.0f + 128.0f);
			const auto zz = uint8_t(_z * 127.0f + 128.0f);
			const auto ww = uint8_t(_w * 127.0f + 128.0f);

			return packUint32(xx, yy, zz, ww);
		}

		static uint32_t packRgba8(float x, float y, float z, float w) {
			return bit_cast<uint32_t>(std::array<uint8_t, 4>{
			    uint8_t(toUnorm(x, 255.0f)),
			    uint8_t(toUnorm(y, 255.0f)),
			    uint8_t(toUnorm(z, 255.0f)),
			    uint8_t(toUnorm(w, 255.0f))});
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
			return PackUtils::packRgba8(
			    _x * 0.5f + 0.5f,
			    _y * 0.5f + 0.5f,
			    _z * 0.5f + 0.5f,
			    _w * 0.5f + 0.5f);
		}
	};
} // namespace rawrBox
