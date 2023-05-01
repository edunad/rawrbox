#pragma once

#include <chrono>

namespace rawrBox {
	class TimeUtils {

	public:
		static int64_t gameTime;

		static float deltaTime;
		static double frameAlpha;

		static std::chrono::high_resolution_clock::time_point startTime;

		static int64_t time();
		static int64_t curTime();
	};
} // namespace rawrBox
