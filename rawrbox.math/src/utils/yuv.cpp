#include <rawrbox/math/utils/yuv.hpp>

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

namespace rawrbox {
	// PRIVATE ---
	rawrbox::YUVUtils::Coefficients YUVUtils::getCoefficients(const rawrbox::YUVImage& image) {
		constexpr double fixedScale = 65536.0;

		double kr = 0.0;
		double kb = 0.0;

		switch (image.space) {
			case rawrbox::YUVColorSpace::RGB:
				break;
			case rawrbox::YUVColorSpace::BT601:
				kr = 0.299;
				kb = 0.114;
				break;
			case rawrbox::YUVColorSpace::BT709:
				kr = 0.2126;
				kb = 0.0722;
				break;
			case rawrbox::YUVColorSpace::BT2020:
				kr = 0.2627;
				kb = 0.0593;
				break;
			case rawrbox::YUVColorSpace::SMPTE240:
				kr = 0.212;
				kb = 0.087;
				break;
			default:
				throw std::runtime_error("Unknown YUV color space");
		}

		const bool full = image.scale == rawrbox::YUVLuminanceScale::FULL;
		const double yScale = full ? 1.0 : 255.0 / 219.0;
		const double cScale = full ? 1.0 : 255.0 / 224.0;
		const double kg = 1.0 - kr - kb;

		const uint32_t depthShift = image.bitDepth - 8U;

		rawrbox::YUVUtils::Coefficients coeff = {};
		coeff.y = static_cast<int32_t>(std::lround(yScale * fixedScale));
		coeff.yOffset = (full ? 0 : 16) << depthShift;
		coeff.cOffset = 128 << depthShift;
		coeff.shift = 16U + depthShift;
		coeff.alphaShift = image.alphaBitDepth - 8U;

		if (image.space != rawrbox::YUVColorSpace::RGB) {
			coeff.crR = static_cast<int32_t>(std::lround(2.0 * (1.0 - kr) * cScale * fixedScale));
			coeff.cbB = static_cast<int32_t>(std::lround(2.0 * (1.0 - kb) * cScale * fixedScale));
			coeff.cbG = static_cast<int32_t>(std::lround(2.0 * kb * (1.0 - kb) / kg * cScale * fixedScale));
			coeff.crG = static_cast<int32_t>(std::lround(2.0 * kr * (1.0 - kr) / kg * cScale * fixedScale));
		}

		return coeff;
	}

	template <typename T>
	void YUVUtils::convertPlanes(const rawrbox::YUVImage& image, const rawrbox::YUVUtils::Coefficients& coeff, uint8_t* dst, uint32_t dstPitch, bool flipY) {
		const bool rgb = image.space == rawrbox::YUVColorSpace::RGB;

		if (image.planes[3] == nullptr) {
			if (rgb) {
				rawrbox::YUVUtils::convertRows<T, void, true>(image, coeff, dst, dstPitch, flipY);
			} else {
				rawrbox::YUVUtils::convertRows<T, void, false>(image, coeff, dst, dstPitch, flipY);
			}
		} else if (image.wideAlpha) {
			if (rgb) {
				rawrbox::YUVUtils::convertRows<T, uint16_t, true>(image, coeff, dst, dstPitch, flipY);
			} else {
				rawrbox::YUVUtils::convertRows<T, uint16_t, false>(image, coeff, dst, dstPitch, flipY);
			}
		} else {
			if (rgb) {
				rawrbox::YUVUtils::convertRows<T, uint8_t, true>(image, coeff, dst, dstPitch, flipY);
			} else {
				rawrbox::YUVUtils::convertRows<T, uint8_t, false>(image, coeff, dst, dstPitch, flipY);
			}
		}
	}

	template <typename T, typename A, bool RGB>
	void YUVUtils::convertRows(const rawrbox::YUVImage& image, const rawrbox::YUVUtils::Coefficients& coeff, uint8_t* dst, uint32_t dstPitch, bool flipY) {
		constexpr bool alpha = !std::is_void_v<A>;
		using AlphaT = std::conditional_t<alpha, A, uint8_t>;

		const uint32_t width = image.size.x;
		const uint32_t height = image.size.y;

		const uint32_t shiftX = image.chromaShift.x;
		const uint32_t shiftY = image.chromaShift.y;

		const uint32_t block = 1U << shiftX;

		const int32_t yScale = coeff.y;

		const int32_t yOffset = coeff.yOffset;
		const int32_t cOffset = RGB ? coeff.yOffset : coeff.cOffset;

		const int32_t rFactor = RGB ? coeff.y : coeff.crR;
		const int32_t bFactor = RGB ? coeff.y : coeff.cbB;

		const int32_t gFactorU = RGB ? 0 : coeff.cbG;
		const int32_t gFactorV = RGB ? 0 : coeff.crG;

		const uint32_t shift = coeff.shift;
		const uint32_t alphaShift = coeff.alphaShift;
		const int32_t round = 1 << (shift - 1U);

		for (uint32_t y = 0; y < height; y++) {
			const auto* yRow = reinterpret_cast<const T*>(image.planes[0] + static_cast<ptrdiff_t>(y) * image.strides[0]);
			const auto* uRow = reinterpret_cast<const T*>(image.planes[1] + static_cast<ptrdiff_t>(y >> shiftY) * image.strides[1]);
			const auto* vRow = reinterpret_cast<const T*>(image.planes[2] + static_cast<ptrdiff_t>(y >> shiftY) * image.strides[2]);

			[[maybe_unused]] const AlphaT* aRow = nullptr;
			if constexpr (alpha) aRow = reinterpret_cast<const AlphaT*>(image.planes[3] + static_cast<ptrdiff_t>(y) * image.strides[3]);

			uint8_t* out = dst + static_cast<size_t>(flipY ? height - 1U - y : y) * dstPitch;

			for (uint32_t x = 0; x < width;) {
				const int32_t u = static_cast<int32_t>(uRow[x >> shiftX]) - cOffset;
				const int32_t v = static_cast<int32_t>(vRow[x >> shiftX]) - cOffset;

				const int32_t rChroma = rFactor * v;
				const int32_t gChroma = gFactorU * u + gFactorV * v;
				const int32_t bChroma = bFactor * u;

				const uint32_t end = width - x > block ? x + block : width;

				for (; x < end; x++) {
					const int32_t lum = yScale * (static_cast<int32_t>(yRow[x]) - yOffset);
					const int32_t base = RGB ? 0 : lum; // GBR

					int32_t b = (base + bChroma + round) >> shift;
					int32_t g = (lum - gChroma + round) >> shift;
					int32_t r = (base + rChroma + round) >> shift;

					b &= ~(b >> 31);
					g &= ~(g >> 31);
					r &= ~(r >> 31);

					out[0] = static_cast<uint8_t>(b | ((255 - b) >> 31));
					out[1] = static_cast<uint8_t>(g | ((255 - g) >> 31));
					out[2] = static_cast<uint8_t>(r | ((255 - r) >> 31));

					if constexpr (alpha) {
						const int32_t a = static_cast<int32_t>(aRow[x]) >> alphaShift;
						out[3] = static_cast<uint8_t>(a | ((255 - a) >> 31));
					} else {
						out[3] = 0xFF;
					}

					out += 4;
				}
			}
		}
	}
	// ------

	void YUVUtils::convert(const rawrbox::YUVImage& image, uint8_t* dst, uint32_t dstPitch, bool flipY) {
		if (dst == nullptr) throw std::runtime_error("Invalid YUV destination");

		if (image.planes[0] == nullptr || image.planes[1] == nullptr || image.planes[2] == nullptr) throw std::runtime_error("Invalid YUV planes");
		if (image.scale == rawrbox::YUVLuminanceScale::UNKNOWN) throw std::runtime_error("Unknown YUV luminance scale");
		if (image.bitDepth < 8U || image.bitDepth > (image.wide ? 12U : 8U)) throw std::runtime_error("Unsupported YUV bit depth");
		if (image.alphaBitDepth < 8U || image.alphaBitDepth > (image.wideAlpha ? 12U : 8U)) throw std::runtime_error("Unsupported YUV alpha bit depth");

		const auto coeff = rawrbox::YUVUtils::getCoefficients(image);
		if (image.wide) {
			rawrbox::YUVUtils::convertPlanes<uint16_t>(image, coeff, dst, dstPitch, flipY);
		} else {
			rawrbox::YUVUtils::convertPlanes<uint8_t>(image, coeff, dst, dstPitch, flipY);
		}
	}
} // namespace rawrbox