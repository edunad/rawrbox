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
	TextureImage::TextureImage(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, int forceChannels, bool useFallback) : _filePath(filePath) {
		uint8_t* image = stbi_load_from_memory(buffer.data(), static_cast<int>(buffer.size()) * sizeof(uint8_t), &this->_size.x, &this->_size.y, &this->_channels, forceChannels);
		if (forceChannels != 0) {
			this->_channels = forceChannels;
		}
		this->internalLoad(image, useFallback);
	}

	TextureImage::TextureImage(const std::filesystem::path& filePath, int forceChannels, bool useFallback) : _filePath(filePath) {
		stbi_uc* image = stbi_load(filePath.generic_string().c_str(), &this->_size.x, &this->_size.y, &this->_channels, forceChannels);
		if (forceChannels != 0) {
			this->_channels = forceChannels;
		}

		this->internalLoad(image, useFallback);
	}

	TextureImage::TextureImage(const uint8_t* buffer, int bufferSize, int forceChannels, bool useFallback) {
		uint8_t* image = stbi_load_from_memory(buffer, bufferSize, &this->_size.x, &this->_size.y, &this->_channels, forceChannels);
		if (forceChannels != 0) {
			this->_channels = forceChannels;
		}
		this->internalLoad(image, useFallback);
	}

	TextureImage::TextureImage(const rawrbox::Vector2i& size, const uint8_t* buffer, int channels) {
		this->_size = size;
		this->_channels = channels;

		this->_pixels.resize(static_cast<uint32_t>(this->_size.x * this->_size.y * channels));
		std::memcpy(this->_pixels.data(), buffer, static_cast<uint32_t>(this->_pixels.size()) * sizeof(uint8_t));

		// Check for transparency ----
		if (this->_channels == 4) {
			for (size_t i = 0; i < this->_pixels.size(); i += this->_channels) {
				if (this->_pixels[i + 3] == 1.F) continue;
				_transparent = true;
				break;
			}
		}
		// ---------------------------
	}

	TextureImage::TextureImage(const rawrbox::Vector2i& size, const std::vector<uint8_t>& buffer, int channels) : TextureImage(size, buffer.data(), channels) {}
	TextureImage::TextureImage(const rawrbox::Vector2i& size, int channels) {
		this->_size = size;
		this->_channels = channels;

		this->_pixels.resize(static_cast<uint32_t>(this->_size.x * this->_size.y * channels));
		std::memset(this->_pixels.data(), 255, this->_pixels.size() * sizeof(uint8_t));
	}

	// NOLINTEND(modernize-pass-by-value)
	void TextureImage::internalLoad(uint8_t* image, bool useFallback) {
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

	// UTILS --------------------
	void TextureImage::updatePixels(const std::vector<uint8_t>& buffer) {
		this->updatePixels(buffer.data(), buffer.size() * sizeof(uint8_t));
	}

	void TextureImage::updatePixels(const uint8_t* buffer, size_t size) {
		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[RawrBox-TextureImage] Failed to bind texture");

		std::memcpy(this->_pixels.data(), buffer, size);
		bgfx::updateTexture2D(this->_handle, 0, 0, 0, 0, static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), bgfx::makeRef(this->_pixels.data(), static_cast<uint32_t>(this->_pixels.size())));
	}

	void TextureImage::setName(const std::string& name) {
		this->_name = name;
	}

	bool TextureImage::hasTransparency() const {
		return this->_channels == 4 && this->_transparent;
	}
	// --------------------

	void TextureImage::upload(bgfx::TextureFormat::Enum format) {
		if (this->_failedToLoad || bgfx::isValid(this->_handle)) return; // Failed texture is already bound, so skip it

		// Try to determine
		if (format == bgfx::TextureFormat::Count) {
			switch (this->_channels) {
				case 1:
					format = bgfx::TextureFormat::R8;
					break;
				case 2:
					format = bgfx::TextureFormat::RG8;
					break;
				case 3:
					format = bgfx::TextureFormat::RGB8;
					break;
				default:
				case 4:
					format = bgfx::TextureFormat::RGBA8;
					break;
			}
		}

		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, format,
		    0 | this->_flags, bgfx::copy(this->_pixels.data(), static_cast<uint32_t>(this->_pixels.size())));

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[RawrBox-TextureImage] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-{}-{}", this->_name, this->_handle.idx).c_str());
	}
} // namespace rawrbox
