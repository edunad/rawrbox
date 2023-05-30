#pragma once
#include <rawrbox/utils/ringbuffer.hpp>
#include <rawrbox/utils/thread_utils.hpp>

#include <fmt/printf.h>

#include <functional>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>

namespace rawrbox {
	class ASYNC {
	protected:
		static bool _shuttingdown;

		static int _workers;
		static int _running;

		static jnk0le::Ringbuffer<std::function<void()>> _threadInvokes;
		static std::exception_ptr _exception_ptr;

		static void completedTask() {
			if (_exception_ptr != nullptr)
				std::rethrow_exception(_exception_ptr);
		}

		static void internal_run(std::function<void()> onComplete = nullptr) {
			if (_running > 0) return; // Already running

			for (size_t i = 0; i < _workers; i++) {
				new std::jthread([onComplete, i]() {
					rawrbox::ThreadUtils::setName("rawrbox:async_#" + std::to_string(i));

					_running++;
					try {
						while (!_threadInvokes.isEmpty() && !_shuttingdown) {
							std::function<void()> fnc;
							_threadInvokes.remove(fnc);

							fnc();
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
						}
					} catch (const std::exception& e) {
						fmt::print("[RawrBox-ASYNC] Fatal error\n  └── {}", e.what());
						_exception_ptr = std::current_exception();
					}

					_running--;
					if (_running <= 0) {
						completedTask();
						if (onComplete != nullptr) onComplete();
					}
				});
			}
		}

	public:
		static void run(std::function<void()> m, std::function<void()> onComplete = nullptr) {
			_threadInvokes.insert(std::move(m));
			internal_run(onComplete);
		}

		static void shutdown() {
			_shuttingdown = true;
			_threadInvokes.consumerClear();
		}
	};
} // namespace rawrbox
