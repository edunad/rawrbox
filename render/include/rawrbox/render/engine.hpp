#pragma once

#include <chrono>
#include <string>

namespace rawrBox {
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
		virtual void init();

		// mark quit flag and allow for `isQuiting()` for clean exit threaded
		virtual void shutdown();

		// poll window and input events
		virtual void pollEvents();

		// called on a fixed timestep
		virtual void update(float deltaTime, int64_t gameTime);

		// called acordingly with the FPS lock
		virtual void draw(const double alpha);

		// starts the game loop and blocks until quit is called and is handled
		virtual void run();

		// sets maximum time of backlog for update and draw calls. Once backlog will be cleared
		// and will start skipping cycles
		virtual void setBreakerTime(const std::chrono::milliseconds& timeBeforeBreaker);
		virtual const std::chrono::milliseconds& getBreakerTime();

		// sets the update rate per second
		virtual void setTPS(uint32_t ticksPerSecond);
		virtual uint32_t getTPS();

		// sets the draw rate per second
		virtual void setFPS(uint32_t framesPerSecond);
		virtual uint32_t getFPS();

		// returns true after quit() is called
		bool isQuitting();

		// are we running behind on updates
		bool isRunningSlow();
	};
} // namespace rawrBox
