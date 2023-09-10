
#include <rawrbox/utils/pack.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>

namespace rawrbox {
	float round(float val, int precision) {
		int c = static_cast<int>(val * std::pow(10, precision) + .5F);
		return c / std::powf(10.F, static_cast<float>(precision));
	}

	uint32_t PackUtils::packUint32(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w) {
		return std::bit_cast<uint32_t>(std::array<uint8_t, 4>{_x, _y, _z, _w});
	}

	uint32_t PackUtils::packF4u(float _x, float _y, float _z, float _w) {
		const auto xx = uint8_t(_x * 127.0F + 128.0F);
		const auto yy = uint8_t(_y * 127.0F + 128.0F);
		const auto zz = uint8_t(_z * 127.0F + 128.0F);
		const auto ww = uint8_t(_w * 127.0F + 128.0F);

		return PackUtils::packUint32(xx, yy, zz, ww);
	}

	uint32_t PackUtils::packRgba8(float x, float y, float z, float w) {
		return std::bit_cast<uint32_t>(std::array<uint8_t, 4>{
		    uint8_t(PackUtils::toUnorm(x, 255.0F)),
		    uint8_t(PackUtils::toUnorm(y, 255.0F)),
		    uint8_t(PackUtils::toUnorm(z, 255.0F)),
		    uint8_t(PackUtils::toUnorm(w, 255.0F))});
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

	uint32_t PackUtils::toAbgr(float _rr, float _gg, float _bb, float _aa) {
		return 0 | (static_cast<uint8_t>(_rr * 255.0F) << 0) | (static_cast<uint8_t>(_gg * 255.0F) << 8) | (static_cast<uint8_t>(_bb * 255.0F) << 16) | (static_cast<uint8_t>(_aa * 255.0F) << 24);
	}
} // namespace rawrbox
