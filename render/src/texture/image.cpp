#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/image.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureImage::TextureImage(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : _filePath(filePath) {
		stbi_uc* image = stbi_load_from_memory(buffer.data(), static_cast<int>(buffer.size()) * sizeof(uint8_t), &this->_size.x, &this->_size.y, &this->_channels, 0);
		this->internalLoad(image, useFallback);
	}

	TextureImage::TextureImage(const std::filesystem::path& filePath, bool useFallback) : _filePath(filePath) {
		stbi_uc* image = stbi_load(filePath.generic_string().c_str(), &this->_size.x, &this->_size.y, &this->_channels, 0);
		this->internalLoad(image, useFallback);
	}
	// NOLINTEND(modernize-pass-by-value)

	void TextureImage::internalLoad(stbi_uc* image, bool useFallback) {
		if (image == nullptr) {
			stbi_image_free(image);

			auto failure = stbi_failure_reason();

			if (useFallback) {
				this->_pixels = rawrbox::MISSING_TEXTURE->pixels;
				this->_channels = 3;
				this->_size = {2, 2};
				this->_failedToLoad = true;

				fmt::print("[TextureImage] Error loading image '{}' | Error: {} --- > Using fallback image\n", this->_filePath.generic_string(), failure);
				return;
			} else {
				throw std::runtime_error(fmt::format("[TextureImage] Error loading image: {}", failure));
			}
		}

		this->_pixels.resize(static_cast<uint32_t>(this->_size.x * this->_size.y) * this->_channels);
		std::memcpy(this->_pixels.data(), image, static_cast<uint32_t>(this->_pixels.size()));

		stbi_image_free(image);
	}

	void TextureImage::setName(const std::string& name) {
		this->_name = name;
	}

	void TextureImage::resize(const rawrbox::Vector2i& newsize) {
		throw std::runtime_error("TODO");
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

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureImage] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-{}-{}", this->_name, this->_handle.idx).c_str());
	}
} // namespace rawrbox
