#pragma once
#include <chrono>

namespace rawrbox {
	struct Watch {
	public:
		std::chrono::high_resolution_clock::time_point timestamp = std::chrono::high_resolution_clock::now();

		[[nodiscard]] inline double elapsed_seconds_since(std::chrono::high_resolution_clock::time_point timestamp2) const {
			const auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timestamp2 - this->timestamp);
			return time_span.count();
		}

		inline double record_elapsed_seconds() {
			const auto timestamp2 = std::chrono::high_resolution_clock::now();
			const auto elapsed = this->elapsed_seconds_since(timestamp2);

			this->timestamp = timestamp2;
			return elapsed;
		}
	};
} // namespace rawrbox