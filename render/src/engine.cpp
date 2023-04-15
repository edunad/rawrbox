#include <rawrbox/render/engine.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std::chrono;
using namespace std::literals;

using Clock = std::chrono::steady_clock;

namespace rawrBox {
	void Engine::init() { throw std::runtime_error("[RawrBox-Engine] Method 'init' not implemented"); }

	void Engine::pollEvents() {}
	void Engine::update(float deltaTime, int64_t gameTime) {}
	void Engine::draw(const double alpha) {}

	void Engine::run() {
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
				update(1.0f / static_cast<float>(this->_tps), (newTime - gameStart).count());
				if (this->_shouldShutdown) return;

				frameTimeTPS -= this->_delayBetweenTicks;
				currentTimeTPS += this->_delayBetweenTicks;

				if (this->_runningSlow) break;
			}

			// we only need to draw a single frame after a update
			// else we're redrawing the same thing without any change
			if (frameTimeFPS >= this->_delayBetweenFrames) {
				draw(1.0f / static_cast<float>(this->_fps));
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

	void Engine::setBreakerTime(const std::chrono::milliseconds& timeBeforeBreaker) {
		this->_deadlockBreaker = timeBeforeBreaker;
	}

	const std::chrono::milliseconds& Engine::getBreakerTime() {
		return this->_deadlockBreaker;
	}

	void Engine::setTPS(uint32_t ticksPerSecond) {
		this->_tps = ticksPerSecond;
		this->_delayBetweenTicks = std::chrono::duration_cast<std::chrono::nanoseconds>(1000ms / this->_tps);
	}

	uint32_t Engine::getTPS() {
		return this->_tps;
	}

	void Engine::setFPS(uint32_t framesPerSeond) {
		this->_fps = framesPerSeond;
		this->_delayBetweenFrames = std::chrono::duration_cast<std::chrono::nanoseconds>(1000ms / this->_fps);
	}

	uint32_t Engine::getFPS() {
		return this->_fps;
	}

	void Engine::shutdown() {
		this->_shouldShutdown = true;
	}

	bool Engine::isQuitting() {
		return this->_shouldShutdown;
	}

	bool Engine::isRunningSlow() {
		return this->_runningSlow;
	}
} // namespace rawrBox
