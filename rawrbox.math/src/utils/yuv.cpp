#include <rawrbox/math/utils/yuv.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

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
	void YUVUtils::convertRows(const rawrbox::YUVImage& image, const rawrbox::YUVUtils::Coefficients& coeff, uint8_t* dst, uint32_t dstPitch, bool flipY) {
		const bool rgb = image.space == rawrbox::YUVColorSpace::RGB;
		const bool hasAlpha = image.planes[3] != nullptr;
		const int32_t round = 1 << (coeff.shift - 1U);

		for (uint32_t y = 0; y < image.size.y; y++) {
			const auto* yRow = reinterpret_cast<const T*>(image.planes[0] + static_cast<ptrdiff_t>(y) * image.strides[0]);
			const auto* uRow = reinterpret_cast<const T*>(image.planes[1] + static_cast<ptrdiff_t>(y >> image.chromaShift.y) * image.strides[1]);
			const auto* vRow = reinterpret_cast<const T*>(image.planes[2] + static_cast<ptrdiff_t>(y >> image.chromaShift.y) * image.strides[2]);

			const uint8_t* aRow = hasAlpha ? image.planes[3] + static_cast<ptrdiff_t>(y) * image.strides[3] : nullptr;
			uint8_t* out = dst + static_cast<size_t>(flipY ? image.size.y - 1U - y : y) * dstPitch;

			for (uint32_t x = 0; x < image.size.x; x++) {
				const int32_t lum = static_cast<int32_t>(yRow[x]) - coeff.yOffset;
				const int32_t cb = static_cast<int32_t>(uRow[x >> image.chromaShift.x]);
				const int32_t cr = static_cast<int32_t>(vRow[x >> image.chromaShift.x]);

				int32_t r = 0;
				int32_t g = 0;
				int32_t b = 0;

				if (rgb) {
					r = coeff.y * (cr - coeff.yOffset);
					g = coeff.y * lum;
					b = coeff.y * (cb - coeff.yOffset);
				} else {
					const int32_t base = coeff.y * lum;
					const int32_t u = cb - coeff.cOffset;
					const int32_t v = cr - coeff.cOffset;

					r = base + coeff.crR * v;
					g = base - coeff.cbG * u - coeff.crG * v;
					b = base + coeff.cbB * u;
				}

				out[0] = static_cast<uint8_t>(std::clamp((b + round) >> coeff.shift, 0, 255));
				out[1] = static_cast<uint8_t>(std::clamp((g + round) >> coeff.shift, 0, 255));
				out[2] = static_cast<uint8_t>(std::clamp((r + round) >> coeff.shift, 0, 255));

				if (hasAlpha) {
					const int32_t alpha = image.wideAlpha ? reinterpret_cast<const uint16_t*>(aRow)[x] : aRow[x];
					out[3] = static_cast<uint8_t>(std::clamp(alpha >> coeff.alphaShift, 0, 255));
				} else {
					out[3] = 0xFF;
				}

				out += 4;
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
			rawrbox::YUVUtils::convertRows<uint16_t>(image, coeff, dst, dstPitch, flipY);
		} else {
			rawrbox::YUVUtils::convertRows<uint8_t>(image, coeff, dst, dstPitch, flipY);
		}
	}
} // namespace rawrbox
