#pragma once
#include <cstdint>
#include <vector>

namespace rawrbox {
	class ZLib {
	public:
		static std::vector<uint8_t> decode(const std::vector<uint8_t>::const_iterator& begin, const std::vector<uint8_t>::const_iterator& end);
		static std::vector<uint8_t> encode(const std::vector<uint8_t>::const_iterator& begin, const std::vector<uint8_t>::const_iterator& end);
	};
} // namespace rawrbox
