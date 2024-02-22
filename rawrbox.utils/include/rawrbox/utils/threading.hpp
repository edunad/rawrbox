#pragma once

#include <rawrbox/utils/logger.hpp>

#include <BS_thread_pool.hpp>

#include <future>

namespace rawrbox {
	class ASYNC {
	protected:
		static std::unique_ptr<BS::thread_pool> _pool;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

	public:
		static void init();
		static void shutdown();

		static std::future<void> run(const std::function<void()>& job);
	};
} // namespace rawrbox
