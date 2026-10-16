#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/utils/logger.hpp>

#include <span>
#include <vector>

namespace rawrbox {
	enum ImageType {
		IMAGE_JPG = 0,
		IMAGE_PNG,
		IMAGE_GIF,
		IMAGE_TIFF,
		IMAGE_BMP,
		IMAGE_WEBP,
		IMAGE_TGA,
		IMAGE_KTX2,
		IMAGE_INVALID,

		COUNT
	};

	struct ImageData;
	class TextureUtils {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static rawrbox::Vector4f atlasToUV(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, uint32_t id);
		static std::vector<uint8_t> generateCheckboard(const rawrbox::Vector2u& size, const rawrbox::Color& color1, const rawrbox::Color& color2, uint32_t amount);

		static std::vector<uint8_t> resize(const rawrbox::Vector2u& originalSize, const std::vector<uint8_t>& data, const rawrbox::Vector2u& newSize, uint8_t channels = 4);

		static std::vector<uint8_t> downsample(const std::vector<uint8_t>& data, const rawrbox::Vector2u& size, uint8_t channels, bool color);

		static float srgbToLinear(uint8_t value);
		static uint8_t linearToSRGB(float value);

		static rawrbox::ImageType getImageType(std::span<const uint8_t> data);
		static rawrbox::ImageData decodeImage(const std::vector<uint8_t>& data);
	};
} // namespace rawrbox