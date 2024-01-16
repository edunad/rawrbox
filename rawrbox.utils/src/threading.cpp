#include <rawrbox/utils/threading.hpp>

#include <cpptrace/cpptrace.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE -------------
	std::unique_ptr<BS::thread_pool> ASYNC::_pool = nullptr;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> ASYNC::_logger = std::make_unique<rawrbox::Logger>("RawrBox-ASYNC");
	// -------------
	// -------------

	void ASYNC::init() {
		if (_pool != nullptr) throw _logger->error("ASYNC init already called!");
		_pool = std::make_unique<BS::thread_pool>(5);
	}

	void ASYNC::shutdown() {
		if (_pool == nullptr) return;

		_pool->purge();
		_pool->reset();
	}

	std::future<void> ASYNC::run(std::function<void()> job) {
		if (_pool == nullptr) throw _logger->error("ASYNC not initialized!");
		std::future<void> future = _pool->submit_task(job);

		try {
			future.get();
		} catch (const cpptrace::exception_with_message& e) {
			throw _logger->error("Fatal error\n  └── {}", e.message());
		}

		return future;
	}
} // namespace rawrbox
