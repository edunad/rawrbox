#include <rawrbox/render/texture/flat.h>

#include <fmt/format.h>

namespace rawrBox {
	TextureFlat::TextureFlat(const rawrBox::Vector2i& initsize, const rawrBox::Color& bgcol) {
		this->_pixels.resize(static_cast<uint32_t>(initsize.y * initsize.x) * this->_channels);
		this->_size = initsize;

		for (size_t i = 0; i < this->_pixels.size(); i += 4) {
			this->_pixels[i] = static_cast<uint8_t>(bgcol.r * 255);
			this->_pixels[i + 1] = static_cast<uint8_t>(bgcol.g * 255);
			this->_pixels[i + 2] = static_cast<uint8_t>(bgcol.b * 255);
			this->_pixels[i + 3] = static_cast<uint8_t>(bgcol.a * 255);
		}
	}

	void TextureFlat::upload(bgfx::TextureFormat::Enum format) {
		if (bgfx::isValid(this->_handle)) return; // Already bound
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, format,
		    BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT, bgfx::copy(this->_pixels.data(), static_cast<uint32_t>(this->_pixels.size())));

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureFlat] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-FLAT-TEXTURE-{}", this->_handle.idx).c_str());
	}
} // namespace rawrBox
