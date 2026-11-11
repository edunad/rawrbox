#include <rawrbox/utils/thread_utils.hpp>
#include <rawrbox/utils/threading.hpp>

#include <fmt/format.h>
#include <tracy/Tracy.hpp>

#include <exception>

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
		if (ASYNC::_pool != nullptr) RAWRBOX_CRITICAL("ASYNC init already called!");
		ASYNC::_pool = std::make_unique<BS::thread_pool<>>(threads, [](size_t index) { rawrbox::ThreadUtils::setName(fmt::format("rawrbox:async:{}", index)); });
	}

	void ASYNC::shutdown() {
		if (ASYNC::_pool == nullptr) return;

		// Stop queued jobs ---
		{
			const std::scoped_lock lock(ASYNC::_lock);
			ASYNC::_closed = true;
		}
		// -----------

		ASYNC::_pool->purge();
		ASYNC::_pool->wait();
	}

	void ASYNC::run(const std::function<void()>& job) {
		if (ASYNC::_pool == nullptr) RAWRBOX_CRITICAL("ASYNC not initialized!");

		const std::scoped_lock lock(ASYNC::_lock);
		if (ASYNC::_closed) return;

		try {
			ASYNC::_pool->detach_task([job]() {
				ZoneScopedN("ASYNC::run");

				try {
					job();
				} catch (const std::exception& e) {
					ASYNC::_logger->error("Fatal error on async job\n  └── {}", e.what());
					spdlog::default_logger()->flush();
					std::terminate();
				}
			});
		} catch (const std::exception& e) {
			RAWRBOX_CRITICAL("Fatal error\n  └── {}", e.what());
		}
	}
} // namespace rawrbox