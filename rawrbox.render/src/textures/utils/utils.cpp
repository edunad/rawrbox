#include <rawrbox/render/textures/utils/utils.hpp>

#include <stdexcept>

namespace rawrbox {
	std::vector<uint8_t> TextureUtils::generateCheckboard(const rawrbox::Vector2u& size, const rawrbox::Color& color1, const rawrbox::Color& color2, uint32_t amount) {
		if (amount % 2 != 0) throw std::runtime_error("Amount must be a multiple of 2.");

		std::vector<uint8_t> pixels = {};

		uint32_t squareSize = std::min(size.x / amount, size.y / amount);
		uint32_t width = squareSize * amount;
		uint32_t height = squareSize * amount;

		pixels.resize(width * height * 4);

		auto cl1 = color1.cast<uint8_t>();
		auto cl2 = color2.cast<uint8_t>();

		uint32_t i = 0;
		for (uint32_t x = 0; x < width; x++) {
			for (uint32_t y = 0; y < height; y++) {
				uint32_t squareX = x / squareSize;
				uint32_t squareY = y / squareSize;

				if ((squareX + squareY) % 2 == 0) {
					pixels[i] = cl1.r;
					pixels[i + 1] = cl1.g;
					pixels[i + 2] = cl1.b;
					pixels[i + 3] = cl1.a;
				} else {
					pixels[i] = cl2.r;
					pixels[i + 1] = cl2.g;
					pixels[i + 2] = cl2.b;
					pixels[i + 3] = cl2.a;
				}

				i += 4;
			}
		}

		return pixels;
	}
} // namespace rawrbox
