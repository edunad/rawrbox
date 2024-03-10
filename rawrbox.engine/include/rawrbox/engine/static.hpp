#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace rawrbox {
	// THREADING ----
	extern std::thread::id RENDER_THREAD_ID;
	extern std::queue<std::function<void()>> RENDER_THREAD_INVOKES;
	extern std::mutex RENDER_THREAD_LOCK;
	// -----

	// TIMING ---
	extern float DELTA_TIME;
	extern float FIXED_DELTA_TIME;
	extern float FRAME_ALPHA;
	// -----

	// NOLINTBEGIN(clang-diagnostic-unused-function)
	static inline void runOnRenderThread(std::function<void()> func) {
		auto id = std::this_thread::get_id();

		if (RENDER_THREAD_ID != id) {
			std::lock_guard<std::mutex> lockGuard(RENDER_THREAD_LOCK);
			RENDER_THREAD_INVOKES.push(std::move(func));
			return;
		}

		func();
	}

	// ⚠️ INTERNAL - DO NOT CALL UNLESS YOU KNOW WHAT YOU ARE DOING ⚠️
	static inline void ___runThreadInvokes() {
		auto id = std::this_thread::get_id();
		if (id != RENDER_THREAD_ID) throw std::runtime_error("Invalid thread, must run on main thread!");

		while (!rawrbox::RENDER_THREAD_INVOKES.empty()) {
			std::function<void()> fnc = nullptr;
			{
				std::lock_guard<std::mutex> lockGuard(RENDER_THREAD_LOCK); // This is a bit slow, having to lock on all the calls, but there's nothing else i can do
				fnc = std::move(rawrbox::RENDER_THREAD_INVOKES.front());
				rawrbox::RENDER_THREAD_INVOKES.pop();
			}

			if (fnc != nullptr) fnc();
		}
	}
	// NOLINTEND(clang-diagnostic-unused-function)
	// -------
} // namespace rawrbox
