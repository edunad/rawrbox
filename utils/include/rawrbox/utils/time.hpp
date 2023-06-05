#pragma once
#include <cstdint>

namespace rawrbox {
	class TimeUtils {
	public:
		static int64_t time();
		static int64_t curtime();
	};
} // namespace rawrbox
