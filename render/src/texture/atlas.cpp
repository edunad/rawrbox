#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/atlas.hpp>
#include <rawrbox/render/utils/texture.hpp>

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
	size_t TextureAtlas::total() const {
		rawrbox::Vector2i totalSprites = this->_size / this->_spriteSize;
		return static_cast<size_t>(totalSprites.x * totalSprites.y);
	}

	uint16_t TextureAtlas::getSpriteSize() const {
		return this->_spriteSize;
	}

	std::vector<uint8_t> TextureAtlas::getSprite(size_t id) const {
		if (id >= this->_tiles.size()) throw std::runtime_error(fmt::format("[RawrBox-Atlas] Invalid ID {}", id));
		return this->_tiles[id];
	}
	// --------------------

	// Adapted from : https://stackoverflow.com/questions/59260533/using-texture-atlas-as-texture-array-in-opengl
	void TextureAtlas::processAtlas() {
		// Setup --
		this->_tiles.clear();
		// -----

		int totalPixels = this->_spriteSize * this->_spriteSize * this->_channels;
		std::vector<uint8_t> tile(totalPixels);

		int tilesX = this->_size.x / this->_spriteSize;
		int tilesY = this->_size.y / this->_spriteSize;

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

				this->_tiles.push_back(tile);
			}
		}

		this->_name = "RawrBox::Texture::ATLAS";
	}

	void TextureAtlas::upload(Diligent::TEXTURE_FORMAT format, bool dynamic) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it

		// Try to determine texture format
		/*if (format == Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN) {
			switch (this->_channels) {
				case 1:
					format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_R8_UNORM;
				case 2:
					format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_RG8_UNORM;
				default:
				case 3:
				case 4:
					format = this->_sRGB ? Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM;
			}
		}

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		desc.Usage = dynamic ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
		desc.Width = this->_spriteSize;
		desc.Height = this->_spriteSize;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.ArraySize = this->total();
		desc.Name = this->_name.c_str();

		std::vector<Diligent::TextureSubResData> subresData(this->total());
		for (uint32_t slice = 0; slice < subresData.size(); ++slice) {
			auto& res = subresData[slice];

			res.pData = this->getSprite(slice).data();
			res.Stride = this->_spriteSize * this->_channels;
		}

		Diligent::TextureData data;
		data.pSubResources = subresData.data();
		data.NumSubresources = subresData.size();

		rawrbox::RENDERER->device->CreateTexture(desc, &data, &this->_tex);
		this->_handle = this->_tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);*/
	}
} // namespace rawrbox
