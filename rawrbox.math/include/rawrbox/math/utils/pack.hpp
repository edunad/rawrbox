#pragma once

#include <array>
#include <cstdint>

namespace rawrbox {
	class PackUtils {
	protected:
		static uint32_t toUnorm(float _value, float _scale);
		static float fromUnorm(uint32_t _value, float _scale);

		static int32_t toSnorm(float _value, float _scale);
		static float fromSnorm(int32_t _value, float _scale);

	public:
		static uint32_t packRgba8(float x, float y, float z, float w);
		static uint32_t packNormal(float _x, float _y = 0.0F, float _z = 0.0F, float _w = 0.0F);

		static std::array<float, 4> fromNormal(uint32_t val);

		static uint16_t toFP16(float half);
		static float fromFP16(uint16_t half);

		static uint32_t toABGR(float _rr, float _gg, float _bb, float _aa);
		static uint32_t toABGR(uint8_t _rr, uint8_t _gg, uint8_t _bb, uint8_t _aa);

		static uint32_t toRGBA(float _rr, float _gg, float _bb, float _aa);
		static uint32_t toRGBA(uint8_t _rr, uint8_t _gg, uint8_t _bb, uint8_t _aa);

		static std::array<float, 4> fromABGR(uint32_t val);
		static std::array<float, 4> fromRGBA(uint32_t val);
		static std::array<float, 4> fromRGB(uint32_t val);
	};
} // namespace rawrbox
