#include <rawrbox/render_temp/static.hpp>
#include <rawrbox/render_temp/texture/atlas.hpp>
#include <rawrbox/render_temp/utils/texture.hpp>

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

	// ------ PIXEL-UTILS
	uint16_t TextureAtlas::total() const {
		rawrbox::Vector2i totalSprites = this->_size / this->_spriteSize;
		return static_cast<uint16_t>(totalSprites.x * totalSprites.y);
	}

	uint16_t TextureAtlas::getSpriteSize() const {
		return this->_spriteSize;
	}

	std::vector<uint8_t> TextureAtlas::getSprite(uint16_t id) const {
		if (id >= this->total()) throw std::runtime_error(fmt::format("[RawrBox-Atlas] Invalid ID {}", id));

		std::vector<uint8_t> pix = {};
		pix.resize(this->_spriteSize * this->_spriteSize * this->_channels);
		auto offset = id * pix.size();

		std::copy(this->_pixels.begin() + offset, this->_pixels.begin() + offset + pix.size(), pix.begin());
		return pix;
	}
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
		    0 | this->_flags, bgfx::copy(this->_pixels.data(), static_cast<uint32_t>(this->_pixels.size())));

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureAtlas] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-ATLAS-{}-{}", this->_name, this->_handle.idx).c_str());
	}
} // namespace rawrbox
