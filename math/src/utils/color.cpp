#include <rawrbox/math/utils/color.hpp>

namespace rawrbox {
	std::vector<uint8_t> ColorUtils::setChannels(size_t channels, size_t target, size_t width, size_t height, const std::vector<uint8_t>& buffer) {
		std::vector<uint8_t> fixedBuffer = {};
		fixedBuffer.resize(width * height * target);

		for (size_t i = 0, j = 0; i < fixedBuffer.size(); i += target, j += channels) {
			std::memcpy(fixedBuffer.data() + i, buffer.data() + j, channels);
		}

		return fixedBuffer;
	}
} // namespace rawrbox
