#include <rawrbox/engine/static.hpp>

namespace rawrbox {
	float DELTA_TIME = 0;       // Update delta time
	float FIXED_DELTA_TIME = 0; // Update fixed delta time
	float FRAME_ALPHA = 0;      // Update delta time

	// THREADING -------
	std::thread::id RENDER_THREAD_ID;
	std::queue<std::function<void()>> RENDER_THREAD_INVOKES = {};
	std::mutex RENDER_THREAD_LOCK;
	// -------

} // namespace rawrbox
