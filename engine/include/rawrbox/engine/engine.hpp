#pragma once

#include <rawrbox/engine/static.hpp>

#include <chrono>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

using namespace std::chrono;
using namespace std::literals;

using Clock = std::chrono::steady_clock;

namespace rawrbox {

	class Engine {
	private:
		bool _shouldShutdown = false;
		bool _runningSlow = false;

		uint32_t _tps = 66;
		uint32_t _fps = 60;

		std::chrono::milliseconds _deadlockBreaker = std::chrono::milliseconds(500);
		std::chrono::nanoseconds _delayBetweenTicks{};
		std::chrono::nanoseconds _delayBetweenFrames{};

	public:
		virtual ~Engine() = default;
		Engine() = default;

		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		// one time initializer
		virtual void init() { throw std::runtime_error("[RawrBox-Engine] Method 'init' not implemented"); };

		// mark quit flag and allow for `isQuiting()` for clean exit threaded
		virtual void shutdown() {
			this->_shouldShutdown = true;
		};

		// poll window and input events
		virtual void pollEvents(){};

		// called on a fixed timestep
		virtual void update(float deltaTime, int64_t gameTime){};

		// called acordingly with the FPS lock
		virtual void draw(){};

		// starts the game loop and blocks until quit is called and is handled
		virtual void run() {
			rawrbox::MAIN_THREAD_ID = std::this_thread::get_id();

			this->_delayBetweenTicks = std::chrono::duration_cast<std::chrono::nanoseconds>(1000ms / this->_tps);
			this->_delayBetweenFrames = std::chrono::duration_cast<std::chrono::nanoseconds>(1000ms / this->_fps);

			// setup init timestamps
			time_point gameStart = Clock::now();
			time_point currentTimeTPS = gameStart;
			time_point currentTimeFPS = gameStart;

			// while game is running, do update and draw logic
			while (!this->_shouldShutdown) {
				// process game input
				pollEvents();

				// track current "tick" time points
				time_point newTime = Clock::now();
				auto frameTimeTPS = newTime - currentTimeTPS;
				auto frameTimeFPS = newTime - currentTimeFPS;

				// Anti spiral of death (mostly when debugging stuff, or game paused / minimized)
				if (frameTimeTPS > this->_deadlockBreaker) frameTimeTPS = this->_deadlockBreaker;
				if (frameTimeFPS > this->_deadlockBreaker) frameTimeFPS = this->_deadlockBreaker;

				// ensure we call update as much times per second as requested
				while (frameTimeTPS >= this->_delayBetweenTicks) {
					update(1.0F / static_cast<float>(this->_tps), (newTime - gameStart).count());
					rawrbox::___runThreadInvokes();
					// THREADING ----
					// -------

					if (this->_shouldShutdown) return;

					frameTimeTPS -= this->_delayBetweenTicks;
					currentTimeTPS += this->_delayBetweenTicks;

					if (this->_runningSlow) break;
				}

				// we only need to draw a single frame after a update
				// else we're redrawing the same thing without any change
				if (frameTimeFPS >= this->_delayBetweenFrames) {
					draw();
					currentTimeFPS = newTime;
				}

				// check if we can go sleep to let the cpu rest
				// or if we need to keep going without sleep to keep up
				newTime = Clock::now();
				frameTimeTPS = newTime - currentTimeTPS;
				if (frameTimeTPS >= this->_delayBetweenTicks) {
					this->_runningSlow = true;
					continue;
				}

				this->_runningSlow = false;
				std::this_thread::sleep_for(1ms);
			}
		}

		// sets maximum time of backlog for update and draw calls. Once backlog will be cleared
		// and will start skipping cycles
		virtual void setBreakerTime(const std::chrono::milliseconds& timeBeforeBreaker) { this->_deadlockBreaker = timeBeforeBreaker; };
		virtual const std::chrono::milliseconds& getBreakerTime() { return this->_deadlockBreaker; };

		// sets the update rate per second
		virtual void setTPS(uint32_t ticksPerSecond) {
			this->_tps = ticksPerSecond;
			this->_delayBetweenTicks = std::chrono::duration_cast<std::chrono::nanoseconds>(1000ms / this->_tps);
		}

		virtual uint32_t getTPS() {
			return this->_tps;
		}

		// sets the draw rate per second
		virtual void setFPS(uint32_t framesPerSecond) {
			this->_fps = framesPerSecond;
			this->_delayBetweenFrames = std::chrono::duration_cast<std::chrono::nanoseconds>(1000ms / this->_fps);
		}

		virtual uint32_t getFPS() {
			return this->_fps;
		}

		// returns true after quit() is called
		bool isQuitting() {
			return this->_shouldShutdown;
		}

		// are we running behind on updates
		bool isRunningSlow() {
			return this->_runningSlow;
		}
	};
} // namespace rawrbox
