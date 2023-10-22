#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/image.hpp>

// NOLINTBEGIN(clang-diagnostic-unknown-pragmas)
#pragma warning(push)
#pragma warning(disable : 4505)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#include <stb/image.hpp>
#pragma warning(pop)
// NOLINTEND(clang-diagnostic-unknown-pragmas)

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureImage::TextureImage(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : _filePath(filePath) {
		uint8_t* image = stbi_load_from_memory(buffer.data(), static_cast<int>(buffer.size()) * sizeof(uint8_t), &this->_size.x, &this->_size.y, &this->_channels, 0);
		this->internalLoad(image, useFallback);
	}

	TextureImage::TextureImage(const std::filesystem::path& filePath, bool useFallback) : _filePath(filePath) {
		stbi_uc* image = stbi_load(filePath.generic_string().c_str(), &this->_size.x, &this->_size.y, &this->_channels, 0);
		this->internalLoad(image, useFallback);
	}

	TextureImage::TextureImage(const uint8_t* buffer, int bufferSize, bool useFallback) {
		uint8_t* image = stbi_load_from_memory(buffer, bufferSize, &this->_size.x, &this->_size.y, &this->_channels, 0);
		this->internalLoad(image, useFallback);
	}

	TextureImage::TextureImage(const rawrbox::Vector2i& size, const uint8_t* buffer, int channels) {
		this->_size = size;
		this->_channels = channels;
		this->_name = "RawrBox::Texture::Image";

		this->_pixels.resize(static_cast<uint32_t>(this->_size.x * this->_size.y * channels));
		std::memcpy(this->_pixels.data(), buffer, static_cast<uint32_t>(this->_pixels.size()) * sizeof(uint8_t));

		// Check for transparency ----
		if (this->_channels == 4) {
			for (size_t i = 0; i < this->_pixels.size(); i += this->_channels) {
				if (this->_pixels[i + 3] == 1.F) continue;
				this->_transparent = true;
				break;
			}
		}
		// ---------------------------
	}

	TextureImage::TextureImage(const rawrbox::Vector2i& size, const std::vector<uint8_t>& buffer, int channels) : TextureImage(size, buffer.data(), channels) {}
	TextureImage::TextureImage(const rawrbox::Vector2i& size, int channels) {
		this->_size = size;
		this->_channels = channels;
		this->_name = "RawrBox::Texture::Image";

		this->_pixels.resize(static_cast<uint32_t>(this->_size.x * this->_size.y * channels));
		std::memset(this->_pixels.data(), 255, this->_pixels.size() * sizeof(uint8_t));
	}

	// NOLINTEND(modernize-pass-by-value)
	void TextureImage::internalLoad(uint8_t* image, bool useFallback) {
		this->_name = "RawrBox::Texture::Image";

		if (image == nullptr) {
			stbi_image_free(image);

			auto failure = stbi_failure_reason();
			if (useFallback) {
				this->loadFallback();
				fmt::print("[RawrBox-TextureImage] Failed to load '{}' ──> {}\n  └── Loading fallback texture!\n", this->_filePath.generic_string(), failure);
				return;
			} else {
				throw std::runtime_error(fmt::format("[RawrBox-TextureImage] Error loading image: {}", failure));
			}
		}

		this->_pixels.resize(static_cast<uint32_t>(this->_size.x * this->_size.y) * this->_channels);
		std::memcpy(this->_pixels.data(), image, static_cast<uint32_t>(this->_pixels.size()) * sizeof(uint8_t));

		// Check for transparency ----
		if (this->_channels == 4) {
			for (size_t i = 0; i < this->_pixels.size(); i += this->_channels) {
				if (this->_pixels[i + 3] == 1.F) continue;
				_transparent = true;
				break;
			}
		}
		// ---------------------------

		stbi_image_free(image);
	}
} // namespace rawrbox
