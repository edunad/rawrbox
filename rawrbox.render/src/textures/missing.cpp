#include <rawrbox/render/textures/missing.hpp>
#include <rawrbox/render/textures/utils/utils.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureMissing::TextureMissing(const rawrbox::Vector2u& size) {
		this->_name = "RawrBox::MISSING";
		this->_data.channels = 4;
		this->_data.size = size;
		this->_data.createFrame(rawrbox::TextureUtils::generateCheckboard(size, rawrbox::Color::RGBHex(0x60005b), rawrbox::Color::RGBHex(0xfe00fc), 2));
	}
} // namespace rawrbox
