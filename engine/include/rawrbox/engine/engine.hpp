#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/engine/threading.hpp>
#include <rawrbox/engine/timer.hpp>

#include <chrono>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

using namespace std::chrono;
using namespace std::literals;

namespace rawrbox {

	class Engine {
	private:
		bool _shouldShutdown = false;
		float _deltaTimeAccumulator = 0;

		uint32_t _tps = 66;
		uint32_t _fps = 60;

		rawrbox::Timer _timer;

		// Quick sleep from  https://github.com/turanszkij/WickedEngine/blob/387c3e0a379a843c433d425f970846a073d55665/WickedEngine/wiApplication.cpp#L148
		void sleep(float milliseconds) {
			const std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
			const double seconds = double(milliseconds) / 1000.0;
			const int sleep_millisec_accuracy = 1;
			const double sleep_sec_accuracy = double(sleep_millisec_accuracy) / 1000.0;

			while (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - t1).count() < seconds) {
				if (seconds - (std::chrono::high_resolution_clock::now() - t1).count() > sleep_sec_accuracy) {
					std::this_thread::sleep_for(std::chrono::milliseconds(sleep_millisec_accuracy));
				}
			}
		}

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
			rawrbox::ASYNC::shutdown();
		};

		// poll window and input events
		virtual void pollEvents(){};

		// called on a fixed timestep
		virtual void fixedUpdate(){};

		// called on a variable timestep
		virtual void update(){};

		// called acordingly with the FPS lock
		virtual void draw(){};

		// starts the game loop and blocks until quit is called and is handled
		virtual void run() {
			rawrbox::MAIN_THREAD_ID = std::this_thread::get_id();
#ifdef _WIN32
			rawrbox::ThreadUtils::setName("RAWRBOX_MAIN_THREAD");
#endif

			while (!this->_shouldShutdown) {
				rawrbox::DELTA_TIME = float(std::max(0.0, this->_timer.record_elapsed_seconds()));

				const float target_deltaTime = 1.0F / this->_fps;
				if (rawrbox::DELTA_TIME < target_deltaTime) {
					sleep((target_deltaTime - rawrbox::DELTA_TIME) * 1000);
					rawrbox::DELTA_TIME += float(std::max(0.0, this->_timer.record_elapsed_seconds()));
				}

				// INPUT
				this->pollEvents();
				// ----------

				// THREADING ----
				rawrbox::___runThreadInvokes();
				// -------

				// Fixed time update --------
				this->_deltaTimeAccumulator += rawrbox::DELTA_TIME;
				if (this->_deltaTimeAccumulator > 10) this->_deltaTimeAccumulator = 0;

				const float targetFrameRateInv = 1.0F / this->_tps;
				while (this->_deltaTimeAccumulator >= targetFrameRateInv) {
					this->fixedUpdate();

					this->_deltaTimeAccumulator -= targetFrameRateInv;
					if (this->_shouldShutdown) return;
				}

				// ---------------------------

				// VARIABLE-TIME
				this->update();
				// ----

				// ACTUAL DRAWING
				rawrbox::FRAME_ALPHA = this->_deltaTimeAccumulator / rawrbox::DELTA_TIME;
				this->draw();
				// ----------
			}
		}
		// sets the update rate per second
		virtual void setTPS(uint32_t ticksPerSecond) {
			this->_tps = ticksPerSecond;
		}

		virtual uint32_t getTPS() {
			return this->_tps;
		}

		// sets the draw rate per second
		virtual void setFPS(uint32_t framesPerSecond) {
			this->_fps = framesPerSecond;
		}

		virtual uint32_t getFPS() {
			return this->_fps;
		}

		// returns true after quit() is called
		bool isQuitting() {
			return this->_shouldShutdown;
		}
	};
} // namespace rawrbox
