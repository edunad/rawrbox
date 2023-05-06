#include <rawrbox/utils/time.hpp>

namespace rawrbox {
	int64_t TimeUtils::gameTime = 0; // Game time since start

	float TimeUtils::deltaTime = 0;   // Update delta time
	double TimeUtils::frameAlpha = 0; // Frame alpha for interpolation

	std::chrono::high_resolution_clock::time_point TimeUtils::startTime;

	int64_t TimeUtils::time() {
		return (std::chrono::high_resolution_clock::now() - startTime).count();
	}

	int64_t TimeUtils::curTime() {
		auto epoch = std::chrono::high_resolution_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
	}
} // namespace rawrbox
