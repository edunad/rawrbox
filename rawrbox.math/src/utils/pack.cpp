
#include <rawrbox/math/utils/pack.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>

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

	uint16_t PackUtils::toFP16(float half) {
		auto bits = std::bit_cast<uint32_t>(half);

		uint32_t sign = (bits >> 31) & 0x1;
		int32_t exponent = ((bits >> 23) & 0xFF) - 127; // Exponent in float32 is biased by 127
		uint32_t mantissa = bits & 0x7FFFFF;

		// Adjust the exponent for FP16's bias (15 instead of 127 for FP32)
		exponent += 15;

		// Handle special cases
		if (exponent >= 31) {
			if (exponent > 31 || mantissa != 0) { // Check for NaN or infinity
				// Preserve the signaling bit in NaNs and set the mantissa to maximum
				return static_cast<uint16_t>((sign << 15) | 0x7C00 | (mantissa != 0U ? 0x200 : 0));
			}

			return static_cast<uint16_t>((sign << 15) | 0x7C00); // Infinity
		}

		if (exponent <= 0) {
			if (exponent < -10) {
				return static_cast<uint16_t>(sign << 15); // Value is too small for FP16 subnormals, flush to zero
			}

			// Handle subnormal numbers
			mantissa |= 0x800000;                                // Add the implicit leading bit
			int shift = 14 - exponent;                           // Calculate how much to shift mantissa to fit in FP16
			mantissa = (mantissa + (1 << (shift - 1))) >> shift; // Round to nearest, ties to even

			return static_cast<uint16_t>((sign << 15) | mantissa);
		}

		// Normalized number
		uint32_t roundedMantissa = (mantissa + 0x00001000) >> 13; // Round to nearest, ties to even
		if ((roundedMantissa & 0x0400) != 0U) {                   // Check if rounding caused a carry
			roundedMantissa = 0;                              // Mantissa overflows to 0
			exponent += 1;                                    // Increase exponent
			if (exponent >= 31) {                             // Check for overflow to infinity
				return static_cast<uint16_t>((sign << 15) | 0x7C00);
			}
		}

		// Pack sign, exponent, and mantissa into FP16 format
		return static_cast<uint16_t>((sign << 15) | (exponent << 10) | (roundedMantissa & 0x03FF));
	}

	float PackUtils::fromFP16(uint16_t half) {
		// Extract the sign (bit 15), exponent (bits 14-10), and mantissa (bits 9-0)
		int sign = (half >> 15) & 0x01;
		int exponent = (half >> 10) & 0x1F;
		int mantissa = half & 0x03FF;

		float signF = sign == 0.F ? 1.0F : -1.0F;
		float mantissaF = mantissa / 1024.0F; // Normalize mantissa

		// Handle special cases for exponent
		if (exponent == 0.F) {
			// Subnormal number (or zero)
			if (mantissa == 0.F) {
				return 0.0F * signF; // +/-0
			}

			// Use exponent of 1 (denormalized number) and adjust mantissa accordingly
			return signF * std::pow(2.F, -14.F) * mantissaF;
		}

		if (exponent == 31.F) {
			// Infinity or NaN (Not a Number)
			if (mantissa == 0.F) {
				return signF * std::numeric_limits<float>::infinity();
			}

			return std::numeric_limits<float>::quiet_NaN();
		}

		// Normalized number
		float exponentF = std::pow(2.F, exponent - 15.F);
		return signF * exponentF * (1.0F + mantissaF);
	}

	uint32_t PackUtils::toABGR(float _rr, float _gg, float _bb, float _aa) {
		return (static_cast<uint8_t>(_rr * 255.0F) << 0) | (static_cast<uint8_t>(_gg * 255.0F) << 8) | (static_cast<uint8_t>(_bb * 255.0F) << 16) | (static_cast<uint8_t>(_aa * 255.0F) << 24);
	}

	uint32_t PackUtils::toABGR(uint8_t _rr, uint8_t _gg, uint8_t _bb, uint8_t _aa) {
		return (_rr << 0) | (_gg << 8) | (_bb << 16) | (_aa << 24);
	}

	uint32_t PackUtils::toRGBA(float _rr, float _gg, float _bb, float _aa) {
		return (static_cast<uint8_t>(_aa * 255.0F) << 0) | (static_cast<uint8_t>(_bb * 255.0F) << 8) | (static_cast<uint8_t>(_gg * 255.0F) << 16) | (static_cast<uint8_t>(_rr * 255.0F) << 24);
	}

	uint32_t PackUtils::toRGBA(uint8_t _rr, uint8_t _gg, uint8_t _bb, uint8_t _aa) {
		return (_aa << 0) | (_bb << 8) | (_gg << 16) | (_rr << 24);
	}

	std::array<float, 4> PackUtils::fromABGR(uint32_t val) {
		return {((val)&0xFF) / 255.0F, ((val >> 8) & 0xFF) / 255.0F, ((val >> 16) & 0xFF) / 255.0F, ((val >> 24) & 0xFF) / 255.0F};
	}

	std::array<float, 4> PackUtils::fromRGBA(uint32_t val) {
		uint8_t r = (val >> 24) & 0xFF;
		uint8_t g = (val >> 16) & 0xFF;
		uint8_t b = (val >> 8) & 0xFF;
		uint8_t a = (val)&0xFF;

		return {
		    r / 255.0F, // Red
		    g / 255.0F, // Green
		    b / 255.0F, // Blue
		    a / 255.0F  // Alpha
		};
	}

	std::array<float, 4> PackUtils::fromRGB(uint32_t val) {
		uint8_t r = (val >> 24) & 0xFF;
		uint8_t g = (val >> 16) & 0xFF;
		uint8_t b = (val >> 8) & 0xFF;

		return {
		    r / 255.0F, // Red
		    g / 255.0F, // Green
		    b / 255.0F, // Blue
		    1.F         // Alpha
		};
	}

} // namespace rawrbox
