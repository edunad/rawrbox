#include <rawrbox/engine/static.hpp>

namespace rawrbox {
	float DELTA_TIME = 0;  // Update delta time
	float FRAME_ALPHA = 0; // Update delta time

	// THREADING -------
	std::thread::id MAIN_THREAD_ID;
	jnk0le::Ringbuffer<std::function<void()>> MAIN_THREAD_INVOKES = {512};
	rawrbox::Threading ASYNC;
	// -------

} // namespace rawrbox
