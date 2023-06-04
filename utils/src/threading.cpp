#include <rawrbox/utils/threading.hpp>

#include <fmt/format.h>

namespace rawrbox {
	BS::thread_pool ASYNC::_pool = {5};

	std::future<void> ASYNC::run(std::function<void()> job) {
		return _pool.submit([job] {
			try {
				job();
			} catch (const std::exception& e) {
				fmt::print("[RawrBox-ASYNC] Fatal error\n  └── {}\n", e.what());
			}
		});
	}

	void ASYNC::shutdown() {
		_pool.purge();
	}

} // namespace rawrbox
