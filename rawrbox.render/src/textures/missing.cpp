#include <rawrbox/render/textures/missing.hpp>
#include <rawrbox/render/textures/utils/utils.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureMissing::TextureMissing(const rawrbox::Vector2u& size) {
		this->setName("MISSING");

		this->_channels = 4;
		this->_size = size;
		this->_pixels = rawrbox::TextureUtils::generateCheckboard(this->_size, rawrbox::Color::RGBHex(0x60005b), rawrbox::Color::RGBHex(0xfe00fc), 2);
	}
} // namespace rawrbox
