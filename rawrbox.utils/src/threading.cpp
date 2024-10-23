#include <rawrbox/utils/threading.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE -------------
	std::unique_ptr<BS::thread_pool> ASYNC::_pool = nullptr;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> ASYNC::_logger = std::make_unique<rawrbox::Logger>("RawrBox-ASYNC");
	// -------------
	// -------------

	void ASYNC::init(uint32_t threads) {
		if (_pool != nullptr) RAWRBOX_CRITICAL("ASYNC init already called!");
		_pool = std::make_unique<BS::thread_pool>(threads);
	}

	void ASYNC::shutdown() {
		if (_pool == nullptr) return;

		_pool->purge();
		_pool->reset();
	}

	void ASYNC::run(const std::function<void()>& job) {
		if (_pool == nullptr) RAWRBOX_CRITICAL("ASYNC not initialized!");

		try {
			_pool->detach_task(job);
		} catch (const std::exception& e) {
			RAWRBOX_CRITICAL("Fatal error\n  └── {}", e.what());
		}
	}
} // namespace rawrbox
