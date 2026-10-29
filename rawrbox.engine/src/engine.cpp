#include <rawrbox/engine/engine.hpp>
#include <rawrbox/engine/static.hpp>
#include <rawrbox/utils/thread_utils.hpp>
#include <rawrbox/utils/threading.hpp>
#include <rawrbox/utils/timer.hpp>

#include <fmt/format.h>
#include <tracy/Tracy.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <thread>

#ifdef _WIN32
	#include <windows.h>
#elif defined(TRACY_ENABLE)
	#include <spawn.h>

extern char** environ;
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
		if (this->_shutdown != ENGINE_THREADS::NONE) return;
		
		rawrbox::ASYNC::shutdown();                      // Finish all jobs
		this->_shutdown = ENGINE_THREADS::THREAD_RENDER; // Stop render first
	}

	void Engine::run() {
		rawrbox::ASYNC::init();
		rawrbox::ThreadUtils::setName("rawrbox:input");

#ifdef TRACY_ENABLE
		// Open profiler window ---
	#ifdef _WIN32
		std::string command = "tracy-profiler.exe -a 127.0.0.1";

		STARTUPINFOA startup = {.cb = sizeof(STARTUPINFOA)};
		PROCESS_INFORMATION process = {};

		if (CreateProcessA(nullptr, command.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, nullptr, &startup, &process) != FALSE) {
			CloseHandle(process.hThread);
			CloseHandle(process.hProcess);
		} else {
			this->_logger->warn("Failed to open 'tracy-profiler.exe', profiling data will be kept in memory until a profiler connects");
		}
	#else
		pid_t pid = 0;
		std::array<std::string, 3> args = {"./tracy-profiler", "-a", "127.0.0.1"};
		std::array<char*, 4> argv = {args[0].data(), args[1].data(), args[2].data(), nullptr};

		if (posix_spawn(&pid, argv[0], nullptr, nullptr, argv.data(), environ) != 0) {
			this->_logger->warn("Failed to open 'tracy-profiler', profiling data will be kept in memory until a profiler connects");
		}
	#endif
		// -----------
#endif

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
					ZoneScopedN("Engine::sleep");
					sleep((target_deltaTime - rawrbox::DELTA_TIME) * 1000);
					rawrbox::DELTA_TIME += static_cast<float>(std::max(0.0, this->_timer.record_elapsed_seconds()));
				}

				// THREADING ----
				{
					ZoneScopedN("Engine::invokes");
					rawrbox::___runThreadInvokes();
				}
				// -------

				// Fixed time update --------
				rawrbox::FIXED_DELTA_TIME = 1.0F / static_cast<float>(this->_tps);
				this->_deltaTimeAccumulator += std::min(rawrbox::DELTA_TIME, MAX_FRAME_TIME);

				while (this->_deltaTimeAccumulator >= rawrbox::FIXED_DELTA_TIME) {
					ZoneScopedN("Engine::fixedUpdate");
					this->fixedUpdate();

					this->_deltaTimeAccumulator -= rawrbox::FIXED_DELTA_TIME;
					if (this->_shutdown != ENGINE_THREADS::NONE) break;
				}

				if (this->_shutdown != ENGINE_THREADS::NONE) break;
				rawrbox::FRAME_ALPHA = std::clamp(this->_deltaTimeAccumulator / rawrbox::FIXED_DELTA_TIME, 0.F, 1.F);
				// ---------------------------

				// VARIABLE-TIME
				{
					ZoneScopedN("Engine::update");
					rawrbox::TIMER::update();
					this->update();
				}
				// ----

				// ACTUAL DRAWING
				{
					ZoneScopedN("Engine::draw");
					this->draw();
				}

				FrameMark;
				// ----------

				// FRAME RATE ---
				this->_frames++;
				this->_framesTime += rawrbox::DELTA_TIME;

				if (this->_framesTime >= 1.F) {
					this->_currentFPS = this->_frames;
					this->_frames = 0;
					this->_framesTime -= 1.F;
				}
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
	uint32_t Engine::getCurrentFPS() const { return this->_currentFPS; }

	bool Engine::isQuitting() const { return this->_shutdown != ENGINE_THREADS::NONE; }
} // namespace rawrbox