
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/atlas.hpp>
#include <rawrbox/render/utils/texture.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureAtlas::TextureAtlas(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, uint32_t spriteSize, bool useFallback) : TextureImage::TextureImage(filePath, buffer, useFallback), _spriteSize(spriteSize) {
		this->processAtlas();
	}

	TextureAtlas::TextureAtlas(const std::filesystem::path& filePath, uint32_t spriteSize, bool useFallback) : TextureImage::TextureImage(filePath, useFallback), _spriteSize(spriteSize) {
		this->processAtlas();
	}
	// NOLINTEND(modernize-pass-by-value)
	// --------------------

	// ------ PIXEL-UTILS
	size_t TextureAtlas::total() const {
		rawrbox::Vector2u totalSprites = this->_size / this->_spriteSize;
		return static_cast<size_t>(totalSprites.x * totalSprites.y);
	}

	uint32_t TextureAtlas::getSpriteSize() const {
		return this->_spriteSize;
	}

	std::vector<uint8_t> TextureAtlas::getSprite(size_t id) const {
		if (id >= this->_tiles.size()) throw this->_logger->error("Invalid ID {}", id);
		return this->_tiles[id];
	}
	// --------------------

	void TextureAtlas::processAtlas() {
		uint32_t tileSizeX = this->_spriteSize * this->_channels;
		uint32_t totalSprites = (this->_size.x / this->_spriteSize) * (this->_size.y / this->_spriteSize);

		this->_tiles.clear();
		this->_tiles.resize(totalSprites);

		for (uint32_t spriteIndex = 0; spriteIndex < totalSprites; ++spriteIndex) {
			uint32_t x = spriteIndex % (this->_size.x / this->_spriteSize);
			uint32_t y = spriteIndex / (this->_size.x / this->_spriteSize);

			auto& pix = this->_tiles[spriteIndex];
			pix.resize(this->_spriteSize * this->_spriteSize * this->_channels);

			for (uint32_t iy = 0; iy < this->_spriteSize; iy++) {
				auto offset = (iy + y * this->_spriteSize) * this->_size.x * this->_channels;
				offset += x * this->_spriteSize * this->_channels;

				std::copy(this->_pixels.begin() + offset, this->_pixels.begin() + offset + tileSizeX, pix.begin() + iy * tileSizeX);
			}
		}

		this->_name = "RawrBox::Texture::ATLAS";
	}

	void TextureAtlas::upload(Diligent::TEXTURE_FORMAT format, bool dynamic) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it

		// Try to determine texture format
		this->tryGetFormatChannels(format, this->_channels);
		// --------------------------------

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		desc.Usage = dynamic ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
		desc.Width = this->_spriteSize;
		desc.Height = this->_spriteSize;
		desc.MipLevels = 1;
		desc.Format = format;
		// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
		desc.ArraySize = static_cast<uint32_t>(this->total());
		// NOLINTEND(cppcoreguidelines-pro-type-union-access)
		desc.Name = this->_name.c_str();

		// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
		std::vector<Diligent::TextureSubResData> subresData(desc.ArraySize);
		// NOLINTEND(cppcoreguidelines-pro-type-union-access)

		for (uint32_t slice = 0; slice < subresData.size(); slice++) {
			auto& res = subresData[slice];

			res.pData = this->_tiles[slice].data();
			res.Stride = desc.Width * this->_channels;
		}

		Diligent::TextureData data;
		data.pSubResources = subresData.data();
		data.NumSubresources = static_cast<uint32_t>(subresData.size());

		rawrbox::RENDERER->device()->CreateTexture(desc, &data, &this->_tex);
		if (this->_tex == nullptr) throw this->_logger->error("Failed to create texture '{}'", this->_name);

		// Get handles --
		this->_handle = this->_tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
		// -------

		rawrbox::BarrierUtils::barrier<Diligent::ITexture>({{this->_tex, Diligent::RESOURCE_STATE_SHADER_RESOURCE}},
		    [this]() {
			    rawrbox::BindlessManager::registerTexture(*this);
		    });
	}
} // namespace rawrbox
