#include <rawrbox/math/utils/color.hpp>

#include <algorithm>

namespace rawrbox {
	std::vector<uint8_t> ColorUtils::setChannels(size_t channels, size_t target, size_t width, size_t height, const std::vector<uint8_t>& buffer) {
		std::vector<uint8_t> fixedBuffer = {};
		fixedBuffer.resize(width * height * target);

		size_t offset = 0;
		for (size_t i = 0; i < buffer.size(); i += channels) {
			for (size_t j = 0; j < target; j++) {
				uint8_t val = j == 3 ? 255 : 1; // A = 255 always
				if (j < channels) {
					val = buffer[i + std::clamp<size_t>(j, 0, channels)];
				}

				fixedBuffer[offset + j] = val;
			}

			offset += target;
		}

		return fixedBuffer;
	}
} // namespace rawrbox
