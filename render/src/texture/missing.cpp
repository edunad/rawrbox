#include <rawrbox/render/texture/missing.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureMissing::TextureMissing() {
		this->_pixels.resize(static_cast<uint32_t>(2 * 2) * 3);
		this->_size = {2, 2};

		// 1e1d1f
		// 4b2b8a
		int i = 0;
		for (int x = 0; x < this->_size.x; x++) {
			for (int y = 0; y < this->_size.y; y++) {
				if ((x == 0 && y == 0) || (x == 1 && y == 1)) {
					this->_pixels[i] = static_cast<uint8_t>(30);
					this->_pixels[i + 1] = static_cast<uint8_t>(29);
					this->_pixels[i + 2] = static_cast<uint8_t>(31);
				} else {
					this->_pixels[i] = static_cast<uint8_t>(75);
					this->_pixels[i + 1] = static_cast<uint8_t>(43);
					this->_pixels[i + 2] = static_cast<uint8_t>(138);
				}

				i += 3;
			}
		}
	}
} // namespace rawrbox
