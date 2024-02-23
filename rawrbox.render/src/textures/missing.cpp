#include <rawrbox/render/textures/missing.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureMissing::TextureMissing() {
		this->setName("MISSING");

		this->_pixels.resize(static_cast<uint32_t>(2 * 2) * 4);
		this->_size = {2, 2};

		// #60005b
		// #fe00fc
		int i = 0;
		for (int x = 0; x < this->_size.x; x++) {
			for (int y = 0; y < this->_size.y; y++) {
				if ((x == 0 && y == 0) || (x == 1 && y == 1)) {
					this->_pixels[i] = static_cast<uint8_t>(96);
					this->_pixels[i + 1] = static_cast<uint8_t>(0);
					this->_pixels[i + 2] = static_cast<uint8_t>(91);
					this->_pixels[i + 3] = static_cast<uint8_t>(255);
				} else {
					this->_pixels[i] = static_cast<uint8_t>(254);
					this->_pixels[i + 1] = static_cast<uint8_t>(0);
					this->_pixels[i + 2] = static_cast<uint8_t>(252);
					this->_pixels[i + 3] = static_cast<uint8_t>(255);
				}

				i += 4;
			}
		}
	}
} // namespace rawrbox
