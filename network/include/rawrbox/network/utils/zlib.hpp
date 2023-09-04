#pragma once
#include <cstdint>
#include <vector>

namespace rawrbox {
	class ZLib {
	public:
		static std::vector<uint8_t> decode(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end);
		static std::vector<uint8_t> encode(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end);
	};
} // namespace rawrbox
