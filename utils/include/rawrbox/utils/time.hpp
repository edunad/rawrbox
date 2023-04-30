#pragma once

#include <chrono>

namespace rawrBox {
	class TimeUtils {

	public:
		// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
		static int64_t gameTime;

		static float deltaTime;
		static double frameAlpha;

		static std::chrono::high_resolution_clock::time_point startTime;
		// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

		static int64_t time();
		static int64_t curTime();
	};
} // namespace rawrBox
