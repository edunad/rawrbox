#pragma once
#include <cstdint>

namespace rawrbox {
	class TimeUtils {
	public:
		static uint64_t time();
		static uint64_t curtime();
	};
} // namespace rawrbox
