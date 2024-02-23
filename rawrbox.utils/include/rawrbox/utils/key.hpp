#pragma once

#include <rawrbox/utils/keys.hpp>

#include <string>

namespace rawrbox {
	class KeyUtils {
	public:
		static std::string keyToStr(uint32_t key);
		static uint32_t strToKey(const std::string& rawKey);
	};
} // namespace rawrbox
