#pragma once
#include <rawrbox/utils/ringbuffer.hpp>

#include <future>
#include <stdexcept>

#ifdef _WIN32
	#include <rawrbox/utils/thread_utils.hpp>
#endif

#include <functional>
#include <string>
#include <thread>

namespace rawrbox {
	class Threading {
	protected:
		bool _shuttingdown = false;

		int _workers = 2;
		int _running = 0;

		jnk0le::Ringbuffer<std::function<void()>> _threadInvokes = {512};
		std::exception_ptr exception_ptr = nullptr;

		void completedTask() {
			if (exception_ptr != nullptr) std::rethrow_exception(exception_ptr);
		}

		void internal_run(std::function<void()> onComplete = nullptr) {
			if (this->_running > 0) return;

			for (size_t i = 0; i < this->_workers; i++) {
				new std::jthread([this, onComplete, i]() {
#ifdef _WIN32
					rawrbox::ThreadUtils::setName("rawrbox:async_#" + std::to_string(i));
#endif
					this->_running++;
					try {
						while (!this->_threadInvokes.isEmpty() && !this->_shuttingdown) {
							std::function<void()> fnc;
							this->_threadInvokes.remove(fnc);

							fnc();
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
						}
					} catch (...) {
						exception_ptr = std::current_exception();
					}

					this->_running--;
					if (this->_running <= 0) {
						this->completedTask();
						if (onComplete != nullptr) onComplete();
					}
				});
			}
		}

	public:
		void run(std::function<void()> m, std::function<void()> onComplete = nullptr) {
			this->_threadInvokes.insert(std::move(m));
			this->internal_run(onComplete);
		}

		void shutdown() {
			this->_shuttingdown = true;
			this->_threadInvokes.consumerClear();
		}
	};
} // namespace rawrbox
