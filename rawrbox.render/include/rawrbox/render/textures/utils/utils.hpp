#pragma once

#include <rawrbox/math/color.hpp>

#include <vector>

namespace rawrbox {
	class TextureUtils {
	public:
		static std::vector<uint8_t> generateCheckboard(const rawrbox::Vector2u& size, const rawrbox::Color& color1, const rawrbox::Color& color2, uint32_t amount);
	};
} // namespace rawrbox
