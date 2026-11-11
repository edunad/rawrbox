#pragma once

#include <rawrbox/utils/logger.hpp>

#include <BS_thread_pool.hpp>
#include <tracy/Tracy.hpp>

#include <exception>
#include <future>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace rawrbox {
	class ASYNC {
	protected:
		static std::unique_ptr<BS::thread_pool<>> _pool;

		static std::mutex _lock;
		static bool _closed;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

	public:
		static void init(uint32_t threads = 0);
		static void shutdown();

		static void run(const std::function<void()>& job);

		template <typename F, typename R = std::invoke_result_t<std::decay_t<F>>>
		[[nodiscard]] static std::future<R> submit(F&& job) {
			if (ASYNC::_pool == nullptr) RAWRBOX_CRITICAL("ASYNC not initialized!");

			const std::scoped_lock lock(ASYNC::_lock);
			if (ASYNC::_closed) {
				std::promise<R> closed;
				closed.set_exception(std::make_exception_ptr(std::runtime_error("ASYNC is shut down")));
				return closed.get_future();
			}

			return ASYNC::_pool->submit_task([task = std::forward<F>(job)]() mutable -> R {
				ZoneScopedN("ASYNC::submit");
				return task();
			});
		}
	};
} // namespace rawrbox
