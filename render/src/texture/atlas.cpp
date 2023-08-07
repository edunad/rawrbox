#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/atlas.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureAtlas::TextureAtlas(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, uint16_t spriteSize, int forceChannels, bool useFallback) : TextureImage::TextureImage(filePath, buffer, forceChannels, useFallback), _spriteSize(spriteSize) {
		this->processAtlas();
	}

	TextureAtlas::TextureAtlas(const std::filesystem::path& filePath, uint16_t spriteSize, int forceChannels, bool useFallback) : TextureImage::TextureImage(filePath, forceChannels, useFallback), _spriteSize(spriteSize) {
		this->processAtlas();
	}
	// NOLINTEND(modernize-pass-by-value)
	// --------------------

	// Adapted from : https://stackoverflow.com/questions/59260533/using-texture-atlas-as-texture-array-in-opengl
	void TextureAtlas::processAtlas() {
		int totalPixels = this->_spriteSize * this->_spriteSize * this->_channels;
		std::vector<uint8_t> tile(totalPixels);

		int tilesX = this->_size.x / this->_spriteSize;
		int tilesY = this->_size.y / this->_spriteSize;

		std::vector<uint8_t> finalPixels = {};

		int tileSizeX = this->_spriteSize * this->_channels;
		int rowLen = tilesX * tileSizeX;

		auto data = this->_pixels.data();
		for (int iy = 0; iy < tilesY; ++iy) {
			for (int ix = 0; ix < tilesX; ++ix) {
				uint8_t* ptr = data + iy * rowLen * this->_spriteSize + ix * tileSizeX;
				for (int row = 0; row < this->_spriteSize; ++row) {
					std::copy(ptr + row * rowLen, ptr + row * rowLen + tileSizeX,
					    tile.begin() + row * tileSizeX);
				}

				finalPixels.insert(finalPixels.end(), tile.begin(), tile.end());
			}
		}

		this->_pixels = finalPixels;
	}

	void TextureAtlas::upload(bgfx::TextureFormat::Enum format) {
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

		rawrbox::Vector2i totalSprites = this->_size / this->_spriteSize;
		this->_handle = bgfx::createTexture2D(this->_spriteSize, this->_spriteSize, false, static_cast<uint16_t>(totalSprites.x * totalSprites.y), format,
		    0 | this->_flags | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP, bgfx::copy(this->_pixels.data(), static_cast<uint32_t>(this->_pixels.size())));

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureAtlas] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-{}-{}", this->_name, this->_handle.idx).c_str());
	}
} // namespace rawrbox
