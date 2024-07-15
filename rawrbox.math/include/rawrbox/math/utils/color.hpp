#pragma once

#include <cstdint>
#include <vector>

namespace rawrbox {
	class ColorUtils {
	public:
		static std::vector<uint8_t> setChannels(uint8_t channels, uint8_t target, size_t width, size_t height, const std::vector<uint8_t>& buffer);
	};
} // namespace rawrbox
