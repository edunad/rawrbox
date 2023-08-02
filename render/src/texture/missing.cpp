#include <rawrbox/render/texture/missing.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureMissing::TextureMissing() {
		this->pixels.resize(static_cast<uint32_t>(2 * 2) * 3);
		this->_size = {2, 2};

		// 1e1d1f
		// 4b2b8a
		int i = 0;
		for (int x = 0; x < this->_size.x; x++) {
			for (int y = 0; y < this->_size.y; y++) {
				if ((x == 0 && y == 0) || (x == 1 && y == 1)) {
					this->pixels[i] = static_cast<uint8_t>(30);
					this->pixels[i + 1] = static_cast<uint8_t>(29);
					this->pixels[i + 2] = static_cast<uint8_t>(31);
				} else {
					this->pixels[i] = static_cast<uint8_t>(75);
					this->pixels[i + 1] = static_cast<uint8_t>(43);
					this->pixels[i + 2] = static_cast<uint8_t>(138);
				}

				i += 3;
			}
		}
	}

	void TextureMissing::upload(bgfx::TextureFormat::Enum format) {
		if (bgfx::isValid(this->_handle)) return; // Already bound

		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, format,
		    0 | this->_flags, bgfx::copy(this->pixels.data(), static_cast<uint32_t>(this->pixels.size())));

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureMissing] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-FLAT-MISSING-{}", this->_handle.idx).c_str());
	}
} // namespace rawrbox
