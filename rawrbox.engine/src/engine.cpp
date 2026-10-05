#include <rawrbox/engine/engine.hpp>
#include <rawrbox/engine/static.hpp>
#include <rawrbox/utils/thread_utils.hpp>
#include <rawrbox/utils/threading.hpp>
#include <rawrbox/utils/timer.hpp>

#include <fmt/format.h>

#include <chrono>
#include <string>
#include <thread>

#ifdef _WIN32
	#include <windows.h>
#endif

namespace rawrbox {
	namespace {
#ifdef _WIN32
		HANDLE hiResTimer() {
			static HANDLE handle = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
			return handle;
		}

		bool preciseSleep(double seconds) {
			HANDLE timer = hiResTimer();
			if (timer == nullptr) return false;

			LARGE_INTEGER due = {};
			due.QuadPart = -static_cast<LONGLONG>(seconds * 1e7);

			if (due.QuadPart >= 0) return true;
			if (SetWaitableTimer(timer, &due, 0, nullptr, nullptr, FALSE) == FALSE) return false;

			WaitForSingleObject(timer, INFINITE);

			return true;
		}
#endif
	} // namespace

	// INTERNAL ---
	void Engine::sleep(float milliseconds) {
		using clock = std::chrono::steady_clock;
		constexpr double SLOP_SEC = 0.001;

		const auto start = clock::now();
		const double target = static_cast<double>(milliseconds) / 1000.0;
		if (target <= 0.0) return;

		for (;;) {
			const double elapsed = std::chrono::duration<double>(clock::now() - start).count();

			const double remaining = target - elapsed;
			if (remaining <= 0.0) break;

			if (remaining > SLOP_SEC) {
#ifdef _WIN32
				if (preciseSleep(remaining - SLOP_SEC)) continue;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
#else
				std::this_thread::sleep_for(std::chrono::duration<double>(remaining - SLOP_SEC));
#endif
				continue;
			}

			std::this_thread::yield();
		}
	}

	// Create the GLFW window
	void Engine::setupGLFW() { RAWRBOX_CRITICAL("Method 'setupGLFW' not implemented"); }
	void Engine::init() {}
	void Engine::pollEvents() {}
	void Engine::fixedUpdate() {}
	void Engine::update() {}
	void Engine::draw() {}

	void Engine::onThreadShutdown(rawrbox::ENGINE_THREADS /*_thread*/) {}
	// -----

	void Engine::shutdown() {
		this->_shutdown = ENGINE_THREADS::THREAD_RENDER; // Stop render first
		rawrbox::ASYNC::shutdown();
	}

	void Engine::run() {
		rawrbox::ASYNC::init();
		rawrbox::ThreadUtils::setName("rawrbox:input");

		// Init GLFW ---
		this->setupGLFW();
		// ---------

		// Setup render threading
		auto renderThread = std::jthread([this]() {
			rawrbox::RENDER_THREAD_ID = std::this_thread::get_id();
			rawrbox::ThreadUtils::setName("rawrbox:render");

			// INITIALIZE ENGINE ---
			this->init();
			// ---------

			while (this->_shutdown != ENGINE_THREADS::THREAD_RENDER) {
				rawrbox::DELTA_TIME = static_cast<float>(std::max(0.0, this->_timer.record_elapsed_seconds()));

				const float target_deltaTime = 1.0F / this->_fps;
				if (rawrbox::DELTA_TIME < target_deltaTime) {
					sleep((target_deltaTime - rawrbox::DELTA_TIME) * 1000);
					rawrbox::DELTA_TIME += static_cast<float>(std::max(0.0, this->_timer.record_elapsed_seconds()));
				}

				// THREADING ----
				rawrbox::___runThreadInvokes();
				// -------

				// Fixed time update --------
				this->_deltaTimeAccumulator += rawrbox::DELTA_TIME;
				if (this->_deltaTimeAccumulator > 10.F) this->_deltaTimeAccumulator = 0; // Prevent dead loop

				const float targetFrameRateInv = 1.0F / this->_tps;
				rawrbox::FIXED_DELTA_TIME = targetFrameRateInv;

				while (this->_deltaTimeAccumulator >= targetFrameRateInv) {
					this->fixedUpdate();

					this->_deltaTimeAccumulator -= targetFrameRateInv;
					if (this->_shutdown != ENGINE_THREADS::NONE) break;
				}

				if (this->_shutdown != ENGINE_THREADS::NONE) break;
				// ---------------------------

				// VARIABLE-TIME
				rawrbox::TIMER::update();
				this->update();
				// ----

				// ACTUAL DRAWING
				rawrbox::FRAME_ALPHA = this->_deltaTimeAccumulator / rawrbox::DELTA_TIME;
				this->draw();
				// ----------
			}

			this->_logger->warn("Thread 'rawrbox:render' shutdown");
			rawrbox::TIMER::clear();

			this->onThreadShutdown(rawrbox::ENGINE_THREADS::THREAD_RENDER);
			this->_shutdown = rawrbox::ENGINE_THREADS::THREAD_INPUT; // Done killing rendering, now destroy glfw
		});
		// ----

		// GLFW needs to run on main thread
		while (this->_shutdown != ENGINE_THREADS::THREAD_INPUT) {
			this->pollEvents();
		}
		// -----

		this->_logger->warn("Thread 'rawrbox:input' shutdown");
		this->onThreadShutdown(rawrbox::ENGINE_THREADS::THREAD_INPUT);
	}

	void Engine::setTPS(uint32_t ticksPerSecond) { this->_tps = ticksPerSecond; }
	uint32_t Engine::getTPS() const { return this->_tps; }

	void Engine::setFPS(uint32_t framesPerSecond) { this->_fps = framesPerSecond; }
	uint32_t Engine::getFPS() const { return this->_fps; }

	bool Engine::isQuitting() const { return this->_shutdown != ENGINE_THREADS::NONE; }
} // namespace rawrbox
