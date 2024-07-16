#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/flat.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureFlat::TextureFlat(const rawrbox::Vector2u& size, const rawrbox::Colorf& bgcol) {
		this->setName("FLAT::" + bgcol.toHEX());

		this->_transparent = bgcol.a != 1.F;
		this->_data.channels = 4;
		this->_data.size = size;

		rawrbox::ImageFrame frame = {};
		frame.pixels.resize(static_cast<uint32_t>(size.y * size.x) * this->_data.channels);
		for (size_t i = 0; i < frame.pixels.size(); i += 4) {
			frame.pixels[i] = static_cast<uint8_t>(bgcol.r * 255);
			frame.pixels[i + 1] = static_cast<uint8_t>(bgcol.g * 255);
			frame.pixels[i + 2] = static_cast<uint8_t>(bgcol.b * 255);
			frame.pixels[i + 3] = static_cast<uint8_t>(bgcol.a * 255);
		}

		this->_data.frames.push_back(frame);
	}

	bool TextureFlat::hasTransparency() const {
		return this->_transparent;
	}
} // namespace rawrbox
