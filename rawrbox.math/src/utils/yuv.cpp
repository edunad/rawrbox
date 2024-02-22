
#include <rawrbox/math/utils/yuv.hpp>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define PUT_PIXEL(s, a, d) \
	L = &rgbToPix[(s)]; \
	*((d)) = L[cb_b]; \
	*((d) + 1) = L[crb_g]; \
	*((d) + 2) = L[cr_r]; \
	*((d) + 3) = (a)
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace rawrbox {

	// PRIVATE ---
	std::vector<int16_t> YUVUtils::_colorTable = {};
	std::array<std::vector<uint8_t>, 2> YUVUtils::_lookupTable = {};
	// ------

	std::vector<int16_t> YUVUtils::getColorTAB() {
		if (!_colorTable.empty()) return _colorTable;
		_colorTable.resize(4 * 256); // R G B A

		int16_t *Cr_r_tab = &_colorTable[0 * 256];
		int16_t *Cr_g_tab = &_colorTable[1 * 256];
		int16_t *Cb_g_tab = &_colorTable[2 * 256];
		int16_t *Cb_b_tab = &_colorTable[3 * 256];

		// Generate the tables for the display surface
		for (int16_t i = 0; i < 256; i++) {
			// Gamma correction (luminescence table) and chroma correction
			// would be done here. See the Berkeley mpeg_play sources.

			int16_t CR = (i - 128);
			int16_t CB = CR;
			Cr_r_tab[i] = (int16_t)((0.419 / 0.299) * CR) + 0 * 768 + 256;
			Cr_g_tab[i] = (int16_t)(-(0.299 / 0.419) * CR) + 1 * 768 + 256;
			Cb_g_tab[i] = (int16_t)(-(0.114 / 0.331) * CB);
			Cb_b_tab[i] = (int16_t)((0.587 / 0.331) * CB) + 2 * 768 + 256;
		}

		return _colorTable;
	}

	std::vector<uint8_t> YUVUtils::lookup(rawrbox::YUVLuminanceScale scale) {
		int a = static_cast<int>(scale);
		if (!_lookupTable[a].empty()) return _lookupTable[a]; // Use the cache

		std::vector<uint8_t> bytes;
		bytes.resize(3 * 768);
		// -----

		uint8_t *r_2_pix_alloc = &bytes[0 * 768];
		uint8_t *g_2_pix_alloc = &bytes[1 * 768];
		uint8_t *b_2_pix_alloc = &bytes[2 * 768];

		if (scale == rawrbox::YUVLuminanceScale::FULL) {
			// Set up entries 0-255 in rgb-to-pixel value tables.
			for (int i = 0; i < 256; i++) {
				r_2_pix_alloc[i + 256] = static_cast<uint8_t>(i);
				g_2_pix_alloc[i + 256] = static_cast<uint8_t>(i);
				b_2_pix_alloc[i + 256] = static_cast<uint8_t>(i);
			}

			// Spread out the values we have to the rest of the array so that we do
			// not need to check for overflow.
			for (int i = 0; i < 256; i++) {
				r_2_pix_alloc[i] = r_2_pix_alloc[256];
				r_2_pix_alloc[i + 512] = r_2_pix_alloc[511];
				g_2_pix_alloc[i] = g_2_pix_alloc[256];
				g_2_pix_alloc[i + 512] = g_2_pix_alloc[511];
				b_2_pix_alloc[i] = b_2_pix_alloc[256];
				b_2_pix_alloc[i + 512] = b_2_pix_alloc[511];
			}
		} else {
			// Set up entries 0-255 in rgb-to-pixel value tables.
			for (int i = 16; i < 236; i++) {
				auto scaledValue = static_cast<uint8_t>((i - 16) * 255 / 219);

				r_2_pix_alloc[i + 256] = scaledValue;
				g_2_pix_alloc[i + 256] = scaledValue;
				b_2_pix_alloc[i + 256] = scaledValue;
			}

			// Spread out the values we have to the rest of the array so that we do
			// not need to check for overflow. We have to do it here in two steps.
			for (int i = 0; i < 256 + 16; i++) {
				r_2_pix_alloc[i] = r_2_pix_alloc[256 + 16];
				g_2_pix_alloc[i] = g_2_pix_alloc[256 + 16];
				b_2_pix_alloc[i] = b_2_pix_alloc[256 + 16];
			}

			for (int i = 256 + 236; i < 768; i++) {
				r_2_pix_alloc[i] = r_2_pix_alloc[256 + 236 - 1];
				g_2_pix_alloc[i] = g_2_pix_alloc[256 + 236 - 1];
				b_2_pix_alloc[i] = b_2_pix_alloc[256 + 236 - 1];
			}
		}

		_lookupTable[a] = bytes; // Cache it
		return bytes;
	}

	void YUVUtils::convert420(rawrbox::YUVLuminanceScale scale, uint8_t *dst, int dstPitch, const uint8_t *ySrc, const uint8_t *uSrc, const uint8_t *vSrc, const uint8_t *aSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
		const auto rgbToPix = lookup(scale);
		const auto colorTab = getColorTAB();

		int halfHeight = yHeight >> 1;
		int halfWidth = yWidth >> 1;

		dst += dstPitch * (yHeight - 2);

		for (int h = 0; h < halfHeight; h++) {
			for (int w = 0; w < halfWidth; w++) {
				const uint8_t *L = nullptr;

				int16_t cr_r = colorTab[*vSrc + 0 * 256];
				int16_t crb_g = colorTab[*vSrc + 1 * 256] + colorTab[*uSrc + 2 * 256];
				int16_t cb_b = colorTab[*uSrc + 3 * 256];

				uSrc++;
				vSrc++;
				PUT_PIXEL(*ySrc, *aSrc, dst + dstPitch);
				PUT_PIXEL(*(ySrc + yPitch), *(aSrc + yPitch), dst);

				ySrc++;
				aSrc++;
				dst += 4;
				PUT_PIXEL(*ySrc, *aSrc, dst + dstPitch);
				PUT_PIXEL(*(ySrc + yPitch), *(aSrc + yPitch), dst);

				ySrc++;
				aSrc++;
				dst += 4;
			}

			dst -= yWidth * 4 + dstPitch * 2;

			ySrc += (yPitch << 1) - yWidth;
			aSrc += (yPitch << 1) - yWidth;
			uSrc += uvPitch - halfWidth;
			vSrc += uvPitch - halfWidth;
		}
	}

	void YUVUtils::convert420(rawrbox::YUVLuminanceScale scale, uint8_t *dst, int dstPitch, const uint8_t *ySrc, const uint8_t *uSrc, const uint8_t *vSrc, int yWidth, int yHeight, int yPitch, int uvPitch) {
		const auto rgbToPix = lookup(scale);
		const auto colorTab = getColorTAB();

		int halfHeight = yHeight >> 1;
		int halfWidth = yWidth >> 1;

		dst += dstPitch * (yHeight - 2);

		for (int h = 0; h < halfHeight; h++) {
			for (int w = 0; w < halfWidth; w++) {
				const uint8_t *L = nullptr;

				int16_t cr_r = colorTab[*vSrc + 0 * 256];
				int16_t crb_g = colorTab[*vSrc + 1 * 256] + colorTab[*uSrc + 2 * 256];
				int16_t cb_b = colorTab[*uSrc + 3 * 256];
				uSrc++;
				vSrc++;

				PUT_PIXEL(*ySrc, 0xFF, dst + dstPitch);
				PUT_PIXEL(*(ySrc + yPitch), 0xFF, dst);
				ySrc++;
				dst += 4;

				PUT_PIXEL(*ySrc, 0xFF, dst + dstPitch);
				PUT_PIXEL(*(ySrc + yPitch), 0xFF, dst);
				ySrc++;
				dst += 4;
			}

			dst -= yWidth * 4 + dstPitch * 2;

			ySrc += (yPitch << 1) - yWidth;
			uSrc += uvPitch - halfWidth;
			vSrc += uvPitch - halfWidth;
		}
	}
} // namespace rawrbox
