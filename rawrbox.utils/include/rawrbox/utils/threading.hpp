#pragma once

#include <rawrbox/utils/logger.hpp>

#include <BS_thread_pool.hpp>

namespace rawrbox {
	class ASYNC {
	protected:
		static std::unique_ptr<BS::thread_pool<>> _pool;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

	public:
		static void init(uint32_t threads = 0);
		static void shutdown();

		static void run(const std::function<void()>& job);
	};
} // namespace rawrbox
