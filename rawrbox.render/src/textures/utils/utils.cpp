#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/utils/gif.hpp>
#include <rawrbox/render/textures/utils/stbi.hpp>
#include <rawrbox/render/textures/utils/utils.hpp>
#include <rawrbox/render/textures/utils/webp.hpp>

#include <magic_enum.hpp>

#include <bitset>

namespace rawrbox {
	// PRIVATE ---
	std::unique_ptr<rawrbox::Logger> TextureUtils::_logger = std::make_unique<rawrbox::Logger>("RawrBox-TextureUtils");
	// ----------

	rawrbox::Vector4f TextureUtils::atlasToUV(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, uint32_t id) {
		// UV -------
		rawrbox::Vector2i totalSprites = atlasSize / spriteSize;
		rawrbox::Vector2f spriteSizeInUV = {static_cast<float>(spriteSize) / atlasSize.x, static_cast<float>(spriteSize) / atlasSize.y};

		uint32_t spriteId = std::clamp<uint32_t>(id, 0, (totalSprites.x * totalSprites.y));

		auto Y = static_cast<uint32_t>(std::floor(spriteId / totalSprites.x));
		auto X = spriteId - Y * totalSprites.x;

		rawrbox::Vector2f uvS = spriteSizeInUV * Vector2f(static_cast<float>(X), static_cast<float>(Y));
		rawrbox::Vector2f uvE = uvS + spriteSizeInUV;

		// -----------
		return {uvS.x, uvS.y, uvE.x, uvE.y};
	};

	std::vector<uint8_t> TextureUtils::generateCheckboard(const rawrbox::Vector2u& size, const rawrbox::Color& color1, const rawrbox::Color& color2, uint32_t amount) {
		if (amount % 2 != 0) RAWRBOX_CRITICAL("Amount must be a power of 2.");
		if (std::bitset<32>(amount).count() != 1) RAWRBOX_CRITICAL("Size must be power of 2.");

		std::vector<uint8_t> pixels = {};

		uint32_t squareSize = std::min(size.x / amount, size.y / amount);
		uint32_t width = squareSize * amount;
		uint32_t height = squareSize * amount;

		pixels.resize(width * height * 4);

		auto cl1 = color1.cast<uint8_t>();
		auto cl2 = color2.cast<uint8_t>();

		uint32_t i = 0;
		for (uint32_t x = 0; x < width; x++) {
			for (uint32_t y = 0; y < height; y++) {
				uint32_t squareX = x / squareSize;
				uint32_t squareY = y / squareSize;

				if ((squareX + squareY) % 2 == 0) {
					pixels[i] = cl1.r;
					pixels[i + 1] = cl1.g;
					pixels[i + 2] = cl1.b;
					pixels[i + 3] = cl1.a;
				} else {
					pixels[i] = cl2.r;
					pixels[i + 1] = cl2.g;
					pixels[i + 2] = cl2.b;
					pixels[i + 3] = cl2.a;
				}

				i += 4;
			}
		}

		return pixels;
	}

	std::vector<uint8_t> TextureUtils::resize(const rawrbox::Vector2u& originalSize, const std::vector<uint8_t>& data, const rawrbox::Vector2u& newSize, uint8_t channels) {
		if (data.empty()) RAWRBOX_CRITICAL("Data cannot be empty");

		std::vector<uint8_t> resizedData(newSize.x * newSize.y * channels); // Assuming RGBA format

		float scaleX = static_cast<float>(originalSize.x) / newSize.x;
		float scaleY = static_cast<float>(originalSize.y) / newSize.y;

		for (uint32_t y = 0; y < newSize.y; ++y) {
			for (uint32_t x = 0; x < newSize.x; ++x) {
				// Calculate corresponding position in original image
				int origX = static_cast<int>(std::round(x * scaleX));
				int origY = static_cast<int>(std::round(y * scaleY));

				// Clamp coordinates to stay within bounds
				origX = std::min(std::max(origX, 0), static_cast<int>(originalSize.x - 1));
				origY = std::min(std::max(origY, 0), static_cast<int>(originalSize.y - 1));

				// Calculate index in original data array
				size_t origIndex = (origY * originalSize.x + origX) * channels;

				// Copy pixel data to resized image
				size_t newIndex = (y * newSize.x + x) * channels;
				std::copy(data.begin() + origIndex, data.begin() + origIndex + channels, resizedData.begin() + newIndex);
			}
		}

		return resizedData;
	}

	rawrbox::ImageType TextureUtils::getImageType(const std::vector<uint8_t>& data) {
		if (data.empty() || data.size() < 16) return rawrbox::ImageType::IMAGE_INVALID;

		// Check for JPEG
		if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
			return rawrbox::ImageType::IMAGE_JPG;
		}

		// Check for PNG
		if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47 && data[4] == 0x0D && data[5] == 0x0A && data[6] == 0x1A && data[7] == 0x0A) {
			return rawrbox::ImageType::IMAGE_PNG;
		}

		// Check for GIF
		if ((data[0] == 'G' && data[1] == 'I' && data[2] == 'F' && data[3] == '8' && (data[4] == '7' || data[4] == '9') && data[5] == 'a')) {
			return rawrbox::ImageType::IMAGE_GIF;
		}

		// Check for TIFF
		if ((data[0] == 0x49 && data[1] == 0x49 && data[2] == 0x2A && data[3] == 0x00) ||
		    (data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2A)) {
			return rawrbox::ImageType::IMAGE_TIFF;
		}

		// Check for BMP
		if (data[0] == 'B' && data[1] == 'M') {
			return rawrbox::ImageType::IMAGE_BMP;
		}

		// Check for WebP
		if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F' && data[8] == 'W' && data[9] == 'E' && data[10] == 'B' && data[11] == 'P') {
			return rawrbox::ImageType::IMAGE_WEBP;
		}

		// Check for TGA
		if (data[2] == 0x02 && data[16] == 0x20 && data[17] == 0x20 && data[18] == 0x20 && data[19] == 0x20) {
			return rawrbox::ImageType::IMAGE_TGA;
		}

		return rawrbox::ImageType::IMAGE_INVALID;
	}

	rawrbox::ImageData TextureUtils::decodeImage(const std::vector<uint8_t>& data) {
		switch (getImageType(data)) {
			case IMAGE_JPG:
			case IMAGE_PNG:
			case IMAGE_TIFF:
			case IMAGE_TGA:
			case IMAGE_BMP:
				return rawrbox::STBI::decode(data);
			case IMAGE_GIF:
				return rawrbox::GIF::decode(data);
			case IMAGE_WEBP:
				return rawrbox::WEBP::decode(data);
			default:
			case IMAGE_INVALID:
				RAWRBOX_CRITICAL("Invalid image type!");
		}
	}
} // namespace rawrbox
