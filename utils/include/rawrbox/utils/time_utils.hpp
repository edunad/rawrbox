#pragma once
#include <chrono>
#include <cstdint>

namespace rawrbox {
	// NOLINTBEGIN(clang-diagnostic-unused-function)
	static int64_t time() {
		static std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
		return (std::chrono::high_resolution_clock::now() - startTime).count();
	}

	static int64_t curtime() {
		auto epoch = std::chrono::high_resolution_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
	}
	// NOLINTEND(clang-diagnostic-unused-function)
} // namespace rawrbox
