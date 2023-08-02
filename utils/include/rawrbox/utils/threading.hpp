#pragma once

#include <BS_thread_pool.hpp>

#include <future>

namespace rawrbox {
	class ASYNC {
	protected:
		static BS::thread_pool _pool;

	public:
		static std::future<void> run(std::function<void()> job);
		static void shutdown();
	};
} // namespace rawrbox
