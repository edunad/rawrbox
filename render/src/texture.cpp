#pragma once

#include <rawrbox/render/texture.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fmt/format.h>

namespace rawrBox {
	Texture::~Texture() {
		bgfx::destroy(this->_handle);
	}

	Texture::Texture(const rawrBox::Vector2i& initsize, const rawrBox::Color& bgcol) {
		this->_pixels = bgfx::alloc(static_cast<uint32_t>(initsize.y * initsize.x) * this->_channels);
		this->_size = initsize;
		this->_originalSize = initsize;

		for (size_t i = 0; i < this->_pixels->size; i+=4) {
			this->_pixels->data[i] = static_cast<uint8_t>(bgcol.r * 255) ;
			this->_pixels->data[i + 1] = static_cast<uint8_t>(bgcol.g * 255);
			this->_pixels->data[i + 2] = static_cast<uint8_t>(bgcol.b * 255);
			this->_pixels->data[i + 3] = static_cast<uint8_t>(bgcol.a * 255);
		}
	}

	Texture::Texture(const std::string& fileName) {
		int w;
		int h;

		stbi_uc *image = stbi_load(fileName.c_str(), &w, &h, &this->_channels, 4); // force it to produce RGBA8
		if(image == NULL) throw std::runtime_error(fmt::format("[Texture] Error loading image: {}", stbi_failure_reason()));

		this->_size = {w, h};
		this->_pixels = bgfx::copy(image, static_cast<uint32_t>(w * h) * this->_channels);

		stbi_image_free(image);
	}

#pragma region PIXEL-UTILS
	rawrBox::Color Texture::getPixel(unsigned int x, unsigned int y) {
		if (this->_pixels->size == 0 && this->_size != 0) {
			throw std::runtime_error("[Texture] Trying to access pixels, but memory is not set");
		}

		size_t index = y * this->_size.x + x;

		rawrBox::Color cl;
		cl.r = this->_pixels->data[index++];
		cl.g = this->_pixels->data[index++];
		cl.b = this->_pixels->data[index++];
		cl.a = this->_pixels->data[index++];

		return cl;
	}

	rawrBox::Color Texture::getPixel(const rawrBox::Vector2i& pos) {
		return this->getPixel(pos.x, pos.y);
	}

	void Texture::setPixel(unsigned int x, unsigned int y, const rawrBox::Color& col) {
		if (this->_pixels->size == 0 && this->_size != 0) {
			throw std::runtime_error("[Texture] Trying to access pixels, but memory is not set");
		}

		size_t index = y * this->_size.x + x;
		this->_pixels->data[index++] = static_cast<uint8_t>(col.r * 255);
		this->_pixels->data[index++] = static_cast<uint8_t>(col.g * 255);
		this->_pixels->data[index++] = static_cast<uint8_t>(col.b * 255);
		this->_pixels->data[index++] = static_cast<uint8_t>(col.a * 255);
	}

	void Texture::setPixel(const rawrBox::Vector2i& pos, const rawrBox::Color& col) {
		this->setPixel(pos.x, pos.y, col);
	}

	void Texture::resize(const rawrBox::Vector2i& newsize) {
		throw std::runtime_error("TODO");
		/*const bgfx::Memory* newpixels = bgfx::alloc(static_cast<uint32_t>(newsize.y * newsize.x) * this->_channels);

		const int x_ratio = (int)((this->_size.x << 16) / newsize.x);
    	const int y_ratio = (int)((this->_size.y << 16) / newsize.y);

		int x_ratio_with_color = x_ratio;

		for (int y = 0; y < newsize.y; y++) {
			int y2_xsource = ((y * y_ratio) >> 16) * this->_size.x;
			int i_xdest = y * newsize.x;

			int source_x_offset = 0;
			int startingOffset = y2_xsource;
			auto inputLine = this->_pixels->data[startingOffset];

			for (int x = 0; x < newsize.x; x++) {
				i_xdest += 1;
				source_x_offset += x_ratio_with_color;
				int sourceOffset = source_x_offset >> 16;

				newpixels->data[i_xdest] = this->_pixels->data[sourceOffset];
			}
		}

		//delete this->_pixels;
		this->_pixels = newpixels;
		this->_size = newsize;*/
	}
	#pragma endregion

	void Texture::upload(int index) {
		if(bgfx::isValid(this->_handle)) return; // Already bound

		this->_index = index;
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, bgfx::TextureFormat::RGBA8,
              BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT
		, this->_pixels);

		if(!bgfx::isValid(this->_handle)) throw std::runtime_error("[Texture] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-TEXTURE-{}-{}", this->_index, this->_handle.idx).c_str());
	}

	bgfx::TextureHandle& Texture::getHandle() {
		return this->_handle;
	}
}
