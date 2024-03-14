#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/flat.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureFlat::TextureFlat(const rawrbox::Vector2u& initsize, const rawrbox::Colorf& bgcol) : _transparent(bgcol.a != 1.F) {
		this->setName("FLAT::" + bgcol.toHEX());
		this->_channels = 4;

		this->_pixels.resize(static_cast<uint32_t>(initsize.y * initsize.x) * this->_channels);
		this->_size = initsize;

		for (size_t i = 0; i < this->_pixels.size(); i += 4) {
			this->_pixels[i] = static_cast<uint8_t>(bgcol.r * 255);
			this->_pixels[i + 1] = static_cast<uint8_t>(bgcol.g * 255);
			this->_pixels[i + 2] = static_cast<uint8_t>(bgcol.b * 255);
			this->_pixels[i + 3] = static_cast<uint8_t>(bgcol.a * 255);
		}
	}

	bool TextureFlat::hasTransparency() const {
		return this->_channels == 4 && this->_transparent;
	}
} // namespace rawrbox
