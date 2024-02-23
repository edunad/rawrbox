
#include <rawrbox/math/utils/pack.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>

namespace rawrbox {
	float round(float val, int precision) {
		int c = static_cast<int>(val * std::pow(10, precision) + .5F);
		return c / std::pow(10.F, static_cast<float>(precision));
	}

	uint32_t PackUtils::packRgba8(float x, float y, float z, float w) {
		return std::bit_cast<uint32_t>(std::array<uint8_t, 4>{
		    uint8_t(PackUtils::toUnorm(x, 255.F)),
		    uint8_t(PackUtils::toUnorm(y, 255.F)),
		    uint8_t(PackUtils::toUnorm(z, 255.F)),
		    uint8_t(PackUtils::toUnorm(w, 255.F))});
	}

	uint32_t PackUtils::packNormal(float _x, float _y, float _z, float _w) {
		return PackUtils::packRgba8(
		    _x * 0.5F + 0.5F,
		    _y * 0.5F + 0.5F,
		    _z * 0.5F + 0.5F,
		    _w * 0.5F + 0.5F);
	}

	std::array<float, 4> PackUtils::fromNormal(uint32_t val) {
		auto arr = std::bit_cast<std::array<uint8_t, 4>>(val);

		float x = PackUtils::fromUnorm(arr[0], 255.0F);
		x = (x - 0.5F) * 2.F;

		float y = PackUtils::fromUnorm(arr[1], 255.0F);
		y = (y - 0.5F) * 2.F;

		float z = PackUtils::fromUnorm(arr[2], 255.0F);
		z = (z - 0.5F) * 2.F;

		float w = PackUtils::fromUnorm(arr[3], 255.0F);
		w = (w - 0.5F) * 2.F;

		return {round(x, 2), round(y, 2), round(z, 2), round(w, 2)};
	}

	uint32_t PackUtils::toUnorm(float _value, float _scale) {
		return uint32_t(std::round(std::clamp(_value, 0.0F, 1.0F) * _scale));
	}

	float PackUtils::fromUnorm(uint32_t _value, float _scale) {
		return static_cast<float>(_value) / _scale;
	}

	int32_t PackUtils::toSnorm(float _value, float _scale) {
		return int32_t(std::round(
		    std::clamp(_value, -1.0F, 1.0F) * _scale));
	}

	float PackUtils::fromSnorm(int32_t _value, float _scale) {
		return std::max(-1.0F, float(_value) / _scale);
	}

	short PackUtils::toHalf(float value) {
		short fltInt16 = 0;
		int fltInt32 = 0;

		std::memcpy(&fltInt32, &value, sizeof(float));
		fltInt16 = ((fltInt32 & 0x7fffffff) >> 13) - (0x38000000 >> 13);
		fltInt16 |= ((fltInt32 & 0x80000000) >> 16);

		return fltInt16;
	}

	float PackUtils::fromHalf(short value) {
		int fltInt32 = ((value & 0x8000) << 16);
		fltInt32 |= ((value & 0x7fff) << 13) + 0x38000000;

		float fRet = 0.F;
		std::memcpy(&fRet, &fltInt32, sizeof(float));
		return fRet;
	}

	uint32_t PackUtils::toABGR(float _rr, float _gg, float _bb, float _aa) {
		return 0 | (static_cast<uint8_t>(_rr * 255.0F) << 0) | (static_cast<uint8_t>(_gg * 255.0F) << 8) | (static_cast<uint8_t>(_bb * 255.0F) << 16) | (static_cast<uint8_t>(_aa * 255.0F) << 24);
	}

	uint32_t PackUtils::toRGBA(float _rr, float _gg, float _bb, float _aa) {
		return 0 | (static_cast<uint8_t>(_aa * 255.0F) << 0) | (static_cast<uint8_t>(_bb * 255.0F) << 8) | (static_cast<uint8_t>(_gg * 255.0F) << 16) | (static_cast<uint8_t>(_rr * 255.0F) << 24);
	}

	std::array<float, 4> PackUtils::fromABGR(uint32_t val) {
		return {((val)&0xFF) / 255.0F, ((val >> 8) & 0xFF) / 255.0F, ((val >> 16) & 0xFF) / 255.0F, ((val >> 24) & 0xFF) / 255.0F};
	}

	std::array<float, 4> PackUtils::fromRGBA(uint32_t val) {
		return {
		    ((val >> 24) & 0xFF) / 255.0F,
		    ((val >> 16) & 0xFF) / 255.0F,
		    ((val >> 8) & 0xFF) / 255.0F,
		    ((val)&0xFF) / 255.0F};
	}

	std::array<float, 4> PackUtils::fromRGB(uint32_t val) {
		return {((val >> 16) & 0xFF) / 255.0F,
		    ((val >> 8) & 0xFF) / 255.0F,
		    ((val)&0xFF) / 255.0F,
		    1.F};
	}

} // namespace rawrbox