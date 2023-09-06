#pragma once
#include <rawrbox/utils/ringbuffer.hpp>

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace rawrbox {
	// THREADING ----
	extern std::thread::id RENDER_THREAD_ID;
	extern std::queue<std::function<void()>> RENDER_THREAD_INVOKES;
	extern std::mutex RENDER_THREAD_LOCK;

	// TIMING ---
	extern float DELTA_TIME;
	extern float FIXED_DELTA_TIME;
	extern float FRAME_ALPHA;
	// -----

	// NOLINTBEGIN(clang-diagnostic-unused-function)
	static inline void runOnRenderThread(std::function<void()> func) {
		if (RENDER_THREAD_ID != std::this_thread::get_id()) {
			std::lock_guard<std::mutex> lock(RENDER_THREAD_LOCK);
			RENDER_THREAD_INVOKES.push(std::move(func));
			return;
		}

		func();
	}

	// ⚠️ INTERNAL - DO NOT CALL UNLESS YOU KNOW WHAT YOU ARE DOING ⚠️
	static inline void ___runThreadInvokes() {
		while (!rawrbox::RENDER_THREAD_INVOKES.empty()) {
			std::function<void()> fnc = std::move(rawrbox::RENDER_THREAD_INVOKES.front());
			rawrbox::RENDER_THREAD_INVOKES.pop();

			fnc();
		}
	}
	// NOLINTEND(clang-diagnostic-unused-function)
	// -------
} // namespace rawrbox
