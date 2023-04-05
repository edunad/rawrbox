#pragma once

#include <rawrbox/render/texture.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fmt/format.h>

namespace rawrBOX {
	Texture::~Texture() {
		bgfx::destroy(this->_handle);
		bgfx::destroy(this->_texColor);
	}

	Texture::Texture(const rawrBOX::Vector2i& initsize, const rawrBOX::Color& bgcol) {
		this->_pixels.resize(initsize.y * initsize.x);
		this->_size = initsize;
		this->_originalSize = initsize;

		for (auto& elm : this->_pixels) elm = bgcol;
	}

	Texture::Texture(const std::string& fileName) {
		int w;
		int h;
		int channels;

		stbi_uc *image = stbi_load(fileName.c_str(), &w, &h, &channels, 4);
		if(image == NULL) throw std::runtime_error(fmt::format("[Texture] Error loading image: {}", stbi_failure_reason()));

		this->_size = {
			static_cast<int>(w),
			static_cast<int>(h)
		};

		this->_originalSize = this->_size;
		this->_pixels.resize(h * w);

		for (unsigned int y = 0; y < static_cast<unsigned int>(h); y++) {
			for (unsigned int x = 0; x < static_cast<unsigned int>(w); x++) {
				int offset = y * w * 4 + x * 4;

				auto& p = this->getPixel(x, y);
				float r = static_cast<float>(image[offset++]) / 255;
				float g = static_cast<float>(image[offset++]) / 255;
				float b = static_cast<float>(image[offset++]) / 255;
				float a = static_cast<float>(image[offset++]) / 255;

				p.r = r * a;
				p.g = g * a;
				p.b = b * a;
				p.a = a;
			}
		}

		stbi_image_free(image);
	}

#pragma region PIXEL-UTILS
	rawrBOX::Color& Texture::getPixel(unsigned int x, unsigned int y) {
		if (this->_pixels.empty() && this->_size != 0) {
			throw std::runtime_error("[Texture] Trying to access pixels, but got empty");
		}

		return this->_pixels[y * this->_size.x + x];
	}

	const rawrBOX::Color& Texture::getPixel(unsigned int x, unsigned int y) const {
		if (this->_pixels.empty() && this->_size != 0) {
			throw std::runtime_error("[Texture] Trying to access pixels, but got empty");
		}

		return this->_pixels[y * this->_size.x + x];
	}

	const rawrBOX::Color& Texture::getPixel(const rawrBOX::Vector2i& pos) const {
		return this->getPixel(pos.x, pos.y);
	}

	rawrBOX::Color& Texture::getPixel(const rawrBOX::Vector2i& pos) {
		return this->getPixel(pos.x, pos.y);
	}

	void Texture::setPixel(const rawrBOX::Vector2i& pos, const rawrBOX::Color& col) {
		this->getPixel(pos) = col;
	}

	void Texture::setPixels(const rawrBOX::Vector2i& size_, const std::vector<rawrBOX::Color>& data) {
		this->_pixels = data;
		this->_size = size_;
	}

	void Texture::resize(const rawrBOX::Vector2i& newsize) {
		std::vector<Color> newpixels;
		newpixels.resize(newsize.y * newsize.x);

		if (!this->_pixels.empty() && this->_size.x > 0 && this->_size.y > 0) {
			auto percentage = newsize.cast<float>() / this->_size.cast<float>();

			for (int cy = 0; cy < newsize.y; cy++) {
				for (int cx = 0; cx < newsize.x; cx++) {
					int pixel = cy * newsize.x + cx;
					int nearestMatch = static_cast<int>(cy / percentage.y) * this->_size.x + static_cast<int>(cx / percentage.x);

					newpixels[pixel] = this->_pixels[nearestMatch];
				}
			}
		}

		this->_size = newsize;
		this->_pixels = newpixels;
	}
	#pragma endregion

	void Texture::bind(int index) {
		if(bgfx::isValid(this->_handle)) return; // Already bound

		// textures must be power of 2
		rawrBOX::Vector2i pow = this->_size;
		if (pow.x != 0 && (pow.x & (pow.x - 1)) != 0) {
			pow.x--;
			pow.x |= pow.x >> 1;
			pow.x |= pow.x >> 2;
			pow.x |= pow.x >> 4;
			pow.x |= pow.x >> 8;
			pow.x |= pow.x >> 16;
			pow.x++;
		}

		if (pow.y != 0 && (pow.y & (pow.y - 1)) != 0) {
			pow.y--;
			pow.y |= pow.y >> 1;
			pow.y |= pow.y >> 2;
			pow.y |= pow.y >> 4;
			pow.y |= pow.y >> 8;
			pow.y |= pow.y >> 16;
			pow.y++;
		}

		if (pow != this->_size) resize(pow);
		// --------------------------------------------

		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), 1 < 0, 1, bgfx::TextureFormat::RGBA32I); // TODO: ADD FLAGS
		if(!bgfx::isValid(this->_handle)) throw std::runtime_error("[Texture] Failed to bind texture");

		bgfx::setName(this->_handle, fmt::format("RAWR-TEXTURE-{}", this->_index).c_str());
		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		this->_index = index;
	}

	void Texture::use() {
		if(!bgfx::isValid(this->_handle)) return;
		bgfx::setTexture(this->_index, this->_texColor, this->_handle);
	}
}
