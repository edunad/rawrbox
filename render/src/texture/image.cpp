#include <rawrbox/render/texture/image.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#include <fmt/format.h>

#include <stb/image.hpp>

namespace rawrBox {
	TextureImage::TextureImage(const std::string& fileName) {
		stbi_uc* image = stbi_load(fileName.c_str(), &this->_size.x, &this->_size.y, &this->_channels, 0);
		if (image == nullptr) throw std::runtime_error(fmt::format("[TextureImage] Error loading image: {}", stbi_failure_reason()));

		this->_pixels.resize(static_cast<uint32_t>(this->_size.x * this->_size.y) * this->_channels);
		std::memcpy(this->_pixels.data(), image, static_cast<uint32_t>(this->_pixels.size()));

		stbi_image_free(image);
	}

	// ------PIXEL-UTILS
	rawrBox::Colori TextureImage::getPixel(unsigned int x, unsigned int y) {
		if (this->_pixels.empty())
			throw std::runtime_error("[TextureImage] Trying to access pixels, but memory is not set");

		size_t index = y * this->_size.x + x;

		rawrBox::Colori cl;
		cl.r = this->_pixels[index++];
		cl.g = this->_pixels[index++];
		cl.b = this->_pixels[index++];
		cl.a = this->_pixels[index++];

		return cl;
	}

	rawrBox::Colori TextureImage::getPixel(const rawrBox::Vector2i& pos) {
		return this->getPixel(pos.x, pos.y);
	}

	void TextureImage::setPixel(unsigned int x, unsigned int y, const rawrBox::Colori& col) {
		if (this->_pixels.empty())
			throw std::runtime_error("[TextureImage] Trying to access pixels, but memory is not set");

		size_t index = y * this->_size.x + x;
		this->_pixels[index++] = static_cast<uint8_t>(col.r);
		this->_pixels[index++] = static_cast<uint8_t>(col.g);
		this->_pixels[index++] = static_cast<uint8_t>(col.b);
		this->_pixels[index++] = static_cast<uint8_t>(col.a);
	}

	void TextureImage::setPixel(const rawrBox::Vector2i& pos, const rawrBox::Colori& col) {
		this->setPixel(pos.x, pos.y, col);
	}

	void TextureImage::resize(const rawrBox::Vector2i& newsize) {
		throw std::runtime_error("TODO");
	}
	// --------------------

	void TextureImage::upload(bgfx::TextureFormat::Enum format) {
		if (bgfx::isValid(this->_handle)) return; // Already bound

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
		    BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT, bgfx::copy(this->_pixels.data(), static_cast<uint32_t>(this->_pixels.size())));

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureImage] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-IMAGE-TEXTURE-{}", this->_handle.idx).c_str());
	}
} // namespace rawrBox
