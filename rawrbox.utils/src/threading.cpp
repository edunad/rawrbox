#include <rawrbox/utils/thread_utils.hpp>
#include <rawrbox/utils/threading.hpp>

#include <fmt/format.h>
#include <tracy/Tracy.hpp>

namespace rawrbox {
	// PRIVATE -------------
	std::unique_ptr<BS::thread_pool<>> ASYNC::_pool = nullptr;

	std::mutex ASYNC::_lock;
	bool ASYNC::_closed = false;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> ASYNC::_logger = std::make_unique<rawrbox::Logger>("RawrBox-ASYNC");
	// -------------
	// -------------

	void ASYNC::init(uint32_t threads) {
		if (_pool != nullptr) RAWRBOX_CRITICAL("ASYNC init already called!");
		_pool = std::make_unique<BS::thread_pool<>>(threads, [](size_t index) { rawrbox::ThreadUtils::setName(fmt::format("rawrbox:async:{}", index)); });
	}

	void ASYNC::shutdown() {
		if (_pool == nullptr) return;

		// Stop queued jobs ---
		{
			const std::scoped_lock lock(_lock);
			_closed = true;
		}
		// -----------

		_pool->purge();
		_pool->wait();
	}

	void ASYNC::run(const std::function<void()>& job) {
		if (_pool == nullptr) RAWRBOX_CRITICAL("ASYNC not initialized!");

		const std::scoped_lock lock(_lock);
		if (_closed) return;

		try {
			_pool->detach_task([job]() {
				ZoneScopedN("ASYNC::run");
				job();
			});
		} catch (const std::exception& e) {
			RAWRBOX_CRITICAL("Fatal error\n  └── {}", e.what());
		}
	}
} // namespace rawrbox