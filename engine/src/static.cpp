#include <rawrbox/engine/static.hpp>

namespace rawrbox {
	// THREADING -------
	std::thread::id MAIN_THREAD_ID;
	jnk0le::Ringbuffer<std::function<void()>> MAIN_THREAD_INVOKES = {64};
	// -------

} // namespace rawrbox
