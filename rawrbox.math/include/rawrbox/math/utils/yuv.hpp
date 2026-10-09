#pragma once

#include <rawrbox/math/vector2.hpp>

#include <array>
#include <cstdint>

namespace rawrbox {

	enum class YUVLuminanceScale : int {
		UNKNOWN = -1,
		FULL = 0, /** Luminance values range from [0, 255] */
		ITU = 1   /** Luminance values range from [16, 235], the range from ITU-R BT.601 */
	};

	enum class YUVColorSpace : int {
		UNKNOWN = -1,
		RGB = 0,
		BT601,
		BT709,
		BT2020,
		SMPTE240
	};

	struct YUVImage {
		std::array<const uint8_t*, 4> planes = {};
		std::array<int, 4> strides = {};

		rawrbox::Vector2u size = {};
		rawrbox::Vector2u chromaShift = {};

		uint32_t bitDepth = 8U;
		uint32_t alphaBitDepth = 8U;

		bool wide = false;
		bool wideAlpha = false;

		rawrbox::YUVLuminanceScale scale = rawrbox::YUVLuminanceScale::UNKNOWN;
		rawrbox::YUVColorSpace space = rawrbox::YUVColorSpace::UNKNOWN;
	};

	class YUVUtils {
		struct Coefficients {
			int32_t y = 0;
			
			int32_t crR = 0;
			int32_t cbG = 0;
			int32_t crG = 0;
			int32_t cbB = 0;

			int32_t yOffset = 0;
			int32_t cOffset = 0;

			uint32_t shift = 0;
			uint32_t alphaShift = 0;
		};

		static rawrbox::YUVUtils::Coefficients getCoefficients(const rawrbox::YUVImage& image);

		template <typename T>
		static void convertRows(const rawrbox::YUVImage& image, const rawrbox::YUVUtils::Coefficients& coeff, uint8_t* dst, uint32_t dstPitch, bool flipY);

	public:
		static void convert(const rawrbox::YUVImage& image, uint8_t* dst, uint32_t dstPitch, bool flipY = false);
	};
} // namespace rawrbox
