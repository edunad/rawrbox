#include <rawrbox/engine/threading.hpp>

namespace rawrbox {
	bool ASYNC::_shuttingdown = false;

	int ASYNC::_workers = 2;
	int ASYNC::_running = 0;

	jnk0le::Ringbuffer<std::function<void()>> ASYNC::_threadInvokes = {512};
	std::exception_ptr ASYNC::_exception_ptr = nullptr;
} // namespace rawrbox
