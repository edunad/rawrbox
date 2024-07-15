#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/utils/utils.hpp>
#include <rawrbox/render/textures/utils/webp.hpp>

#include <bitset>

// NOLINTBEGIN(clang-diagnostic-unknown-pragmas)
#pragma warning(push)
#pragma warning(disable : 4505)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#include <stb/stb_image.hpp>
#pragma warning(pop)
// NOLINTEND(clang-diagnostic-unknown-pragmas)

namespace rawrbox {
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
		if (amount % 2 != 0) throw rawrbox::Logger::err("TextureUtils", "Amount must be a power of 2.");
		if (std::bitset<32>(amount).count() != 1) throw rawrbox::Logger::err("TextureUtils", "Size must be power of 2.");

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
		if (data.empty()) throw rawrbox::Logger::err("TextureUtils", "Data cannot be empty");

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

	// .jpg:  FF D8 FF
	// .png:  89 50 4E 47 0D 0A 1A 0A
	// .gif:  GIF87a
	//        GIF89a
	// .tiff: 49 49 2A 00
	//        4D 4D 00 2A
	// .bmp:  BM
	// .webp: RIFF ???? WEBP
	// .ico   00 00 01 00
	//        00 00 02 00 ( cursor files )
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

		// Check for ICO
		if ((data[0] == 0x00 && data[1] == 0x00 && (data[2] == 0x01 || data[2] == 0x02) && data[3] == 0x00)) {
			return rawrbox::ImageType::IMAGE_ICO;
		}

		return rawrbox::ImageType::IMAGE_INVALID;
	}

	rawrbox::ImageData TextureUtils::decodeImage(const std::vector<uint8_t>& data) {
		rawrbox::ImageType type = getImageType(data);
		if (type == rawrbox::ImageType::IMAGE_INVALID) throw rawrbox::Logger::err("TextureUtils", "Invalid image type!");
		if (type == rawrbox::ImageType::IMAGE_WEBP) return rawrbox::WEBP::decode(data);

		int width = 0;
		int height = 0;
		int channels = 0;

		uint8_t* image = stbi_load_from_memory(data.data(), static_cast<int>(data.size()) * sizeof(uint8_t), &width, &height, &channels, 0);
		if (image == nullptr) throw rawrbox::Logger::err("TextureUtils", "Error decoding image!");

		rawrbox::ImageData texture = {};
		texture.channels = static_cast<uint8_t>(channels);
		texture.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		rawrbox::ImageFrame frame = {};
		frame.pixels.resize(width * height * channels);
		std::memcpy(frame.pixels.data(), image, width * height * channels);

		texture.frames.push_back(frame);
		stbi_image_free(image);

		return texture;
	}
} // namespace rawrbox
