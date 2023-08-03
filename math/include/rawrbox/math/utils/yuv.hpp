#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace rawrbox {

	enum class YUVLuminanceScale : int {
		UNKNOWN = -1,
		FULL = 0, /** Luminance values range from [0, 255] */
		ITU = 1   /** Luminance values range from [16, 235], the range from ITU-R BT.601 */
	};

	class YUVUtils {
		static std::vector<int16_t> _colorTable;
		static std::array<std::vector<uint8_t>, 2> _lookupTable;

	public:
		static std::vector<int16_t> getColorTAB();
		static std::vector<uint8_t> lookup(rawrbox::YUVLuminanceScale scale);

		static void convert420(rawrbox::YUVLuminanceScale scale, uint8_t *dst, int dstPitch, const uint8_t *ySrc, const uint8_t *uSrc, const uint8_t *vSrc, const uint8_t *aSrc, int yWidth, int yHeight, int yPitch, int uvPitch);
		static void convert420(rawrbox::YUVLuminanceScale scale, uint8_t *dst, int dstPitch, const uint8_t *ySrc, const uint8_t *uSrc, const uint8_t *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch);
	};
} // namespace rawrbox
