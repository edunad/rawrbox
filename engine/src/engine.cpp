#include <rawrbox/engine/engine.hpp>
#include <rawrbox/engine/static.hpp>
#include <rawrbox/utils/thread_utils.hpp>
#include <rawrbox/utils/timer.hpp>

#include <cpptrace/cpptrace.hpp>

#include <fmt/printf.h>

#include <chrono>
#include <string>
#include <thread>

namespace rawrbox {
	// INTERNAL ---
	void Engine::sleep(float milliseconds) {
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

	// Create the GLFW window
	void Engine::setupGLFW() { throw cpptrace::logic_error("[RawrBox-Engine] Method 'setupGLFW' not implemented"); }
	void Engine::init() {}
	void Engine::pollEvents() {}
	void Engine::fixedUpdate() {}
	void Engine::update() {}
	void Engine::draw() {}

	void Engine::onThreadShutdown(rawrbox::ENGINE_THREADS /*_thread*/) {}
	// -----

	void Engine::shutdown() {
		this->_shutdown = ENGINE_THREADS::THREAD_RENDER; // Stop bgfx first
	}

	void Engine::run() {

		rawrbox::ThreadUtils::setName("rawrbox:input");
		this->setupGLFW();

		// Setup render threading
		new std::jthread([this]() {
			try {
				rawrbox::RENDER_THREAD_ID = std::this_thread::get_id();
				this->init();

				rawrbox::ThreadUtils::setName("rawrbox:render");
				while (this->_shutdown != ENGINE_THREADS::THREAD_RENDER) {
					rawrbox::DELTA_TIME = float(std::max(0.0, this->_timer.record_elapsed_seconds()));

					const float target_deltaTime = 1.0F / this->_fps;
					if (rawrbox::DELTA_TIME < target_deltaTime) {
						sleep((target_deltaTime - rawrbox::DELTA_TIME) * 1000);
						rawrbox::DELTA_TIME += float(std::max(0.0, this->_timer.record_elapsed_seconds()));
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

				fmt::print("[RawrBox-Engine] Thread 'rawrbox:render' shutdown\n");
				rawrbox::TIMER::clear();
				this->onThreadShutdown(rawrbox::ENGINE_THREADS::THREAD_RENDER);

				this->_shutdown = rawrbox::ENGINE_THREADS::THREAD_INPUT; // Done killing bgfx, now destroy glfw
			} catch (std::exception& err) {
				std::string wat = err.what();

				std::string title = " FATAL ENGINE ERROR ";
				std::string hLine = std::string(((wat.size() / 2) - title.size() / 2), '-');

				fmt::print("\n┌{}{}{}┐\n", hLine, title, hLine);
				fmt::print(" {}\n", wat);
				fmt::print("└{}{}{}┘\n\n", hLine, title, hLine);

				cpptrace::generate_trace().print();
				throw err;
			}
		});

		// ----

		// GLFW needs to run on main thread
		while (this->_shutdown != ENGINE_THREADS::THREAD_INPUT) {
			this->pollEvents();
		}
		// -----

		fmt::print("[RawrBox-Engine] Thread 'rawrbox:input' shutdown\n");
		this->onThreadShutdown(rawrbox::ENGINE_THREADS::THREAD_INPUT);
	}

	void Engine::setTPS(uint32_t ticksPerSecond) { this->_tps = ticksPerSecond; }
	uint32_t Engine::getTPS() const { return this->_tps; }

	void Engine::setFPS(uint32_t framesPerSecond) { this->_fps = framesPerSecond; }
	uint32_t Engine::getFPS() const { return this->_fps; }

	bool Engine::isQuitting() const { return this->_shutdown != ENGINE_THREADS::NONE; }
} // namespace rawrbox
