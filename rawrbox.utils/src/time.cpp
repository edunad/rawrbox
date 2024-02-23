
#include <rawrbox/utils/time.hpp>

#include <chrono>

namespace rawrbox {
	// NOLINTBEGIN(clang-diagnostic-unused-function)
	uint64_t TimeUtils::time() {
		static std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::high_resolution_clock::now() - startTime)).count();
	}

	uint64_t TimeUtils::curtime() {
		auto epoch = std::chrono::high_resolution_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
	}
	// NOLINTEND(clang-diagnostic-unused-function)
} // namespace rawrbox
