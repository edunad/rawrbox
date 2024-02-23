#pragma once

#include <cstdint>
#include <vector>

namespace rawrbox {
	class ColorUtils {
	public:
		static std::vector<uint8_t> setChannels(size_t channels, size_t target, size_t width, size_t height, const std::vector<uint8_t>& buffer);
	};
} // namespace rawrbox
