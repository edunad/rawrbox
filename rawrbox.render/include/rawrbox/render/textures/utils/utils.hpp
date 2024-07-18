#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector4.hpp>

#include <vector>

namespace rawrbox {
	enum ImageType {
		IMAGE_JPG,     // joint photographic experts group - .jpeg or .jpg
		IMAGE_PNG,     // portable network graphics
		IMAGE_GIF,     // graphics interchange format
		IMAGE_TIFF,    // tagged image file format
		IMAGE_BMP,     // Microsoft bitmap format
		IMAGE_WEBP,    // Google WebP format, a type of .riff file
		IMAGE_TGA,     // Truevision Targa image
		IMAGE_INVALID, // unidentified image types.
	};

	struct ImageData;
	class TextureUtils {
	public:
		static rawrbox::Vector4f atlasToUV(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, uint32_t id);
		static std::vector<uint8_t> generateCheckboard(const rawrbox::Vector2u& size, const rawrbox::Color& color1, const rawrbox::Color& color2, uint32_t amount);

		static std::vector<uint8_t> resize(const rawrbox::Vector2u& originalSize, const std::vector<uint8_t>& data, const rawrbox::Vector2u& newSize, uint8_t channels = 4);

		static rawrbox::ImageType getImageType(const std::vector<uint8_t>& data);
		static rawrbox::ImageData decodeImage(const std::vector<uint8_t>& data);
	};
} // namespace rawrbox
