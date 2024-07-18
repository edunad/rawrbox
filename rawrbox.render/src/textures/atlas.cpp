
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/textures/atlas.hpp>
#include <rawrbox/render/textures/utils/stbi.hpp>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureAtlas::TextureAtlas(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, uint32_t spriteSize, bool useFallback) : _spriteSize(spriteSize) {
		try {
			this->processAtlas(rawrbox::STBI::decode(buffer));
		} catch (const cpptrace::exception_with_message& e) {
			if (useFallback) {
				this->loadFallback();
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", filePath.generic_string(), e.what());
				return;
			}

			throw e;
		}
	}

	TextureAtlas::TextureAtlas(const std::filesystem::path& filePath, uint32_t spriteSize, bool useFallback) : _spriteSize(spriteSize) {
		try {
			this->processAtlas(rawrbox::STBI::decode(filePath));
		} catch (const cpptrace::exception_with_message& e) {
			if (useFallback) {
				this->loadFallback();
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", filePath.generic_string(), e.what());
				return;
			}

			throw e;
		}
	}
	// NOLINTEND(modernize-pass-by-value)
	// --------------------

	// ------ PIXEL-UTILS
	size_t TextureAtlas::total() const {
		return this->_data.total();
	}

	uint32_t TextureAtlas::getSpriteSize() const {
		return this->_spriteSize;
	}

	std::vector<uint8_t> TextureAtlas::getSprite(size_t id) const {
		if (id >= this->_data.frames.size()) throw this->_logger->error("Invalid ID {}", id);
		return this->_data.frames[id].pixels;
	}
	// --------------------

	void TextureAtlas::processAtlas(const rawrbox::ImageData& data) {
		if (!data.valid() || data.total() == 0) throw this->_logger->error("Invalid image data!");

		const auto& pixels = data.frames[0].pixels;

		this->_data.channels = data.channels;
		this->_data.size = {this->_spriteSize, this->_spriteSize};

		uint32_t tileSizeX = this->_spriteSize * this->_data.channels;
		uint32_t totalSprites = (data.size.x / this->_spriteSize) * (data.size.y / this->_spriteSize);

		this->_data.frames.resize(totalSprites);

		for (uint32_t spriteIndex = 0; spriteIndex < totalSprites; ++spriteIndex) {
			uint32_t x = spriteIndex % (data.size.x / this->_spriteSize);
			uint32_t y = spriteIndex / (data.size.x / this->_spriteSize);

			auto& pix = this->_data.frames[spriteIndex].pixels;
			pix.resize(this->_spriteSize * this->_spriteSize * data.channels);

			for (uint32_t iy = 0; iy < this->_spriteSize; iy++) {
				auto offset = (iy + y * this->_spriteSize) * data.size.x * data.channels;
				offset += x * this->_spriteSize * data.channels;

				std::copy(pixels.begin() + offset, pixels.begin() + offset + tileSizeX, pix.begin() + iy * tileSizeX);
			}
		}

		this->_name = "RawrBox::Texture::ATLAS";
	}
} // namespace rawrbox
