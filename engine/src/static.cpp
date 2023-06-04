#include <rawrbox/engine/static.hpp>

namespace rawrbox {
	float DELTA_TIME = 0;       // Update delta time
	float FIXED_DELTA_TIME = 0; // Update fixed delta time
	float FRAME_ALPHA = 0;      // Update delta time

	// THREADING -------
	std::thread::id RENDER_THREAD_ID;
	jnk0le::Ringbuffer<std::function<void()>> RENDER_THREAD_INVOKES = {512};
	// -------

} // namespace rawrbox
