#pragma once

#include <array>
#include <cstdint>

// From https://github.com/bkaradzic/bgfx/blob/master/examples/33-pom/pom.cpp#L56
namespace rawrbox {
	class PackUtils {
	public:
		static uint32_t packUint32(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w);
		static uint32_t packF4u(float _x, float _y = 0.0F, float _z = 0.0F, float _w = 0.0F);
		static uint32_t packRgba8(float x, float y, float z, float w);
		static uint32_t packNormal(float _x, float _y = 0.0F, float _z = 0.0F, float _w = 0.0F);
		static std::array<float, 4> fromNormal(uint32_t val);

		static uint32_t toUnorm(float _value, float _scale);
		static float fromUnorm(uint32_t _value, float _scale);

		static int32_t toSnorm(float _value, float _scale);
		static float fromSnorm(int32_t _value, float _scale);

		static uint32_t toABGR(float _rr, float _gg, float _bb, float _aa);
		static uint32_t toRGBA(float _rr, float _gg, float _bb, float _aa);
		static std::array<float, 4> fromRGBA(uint32_t val);
		static std::array<float, 4> fromRGB(uint32_t val);
	};
} // namespace rawrbox
