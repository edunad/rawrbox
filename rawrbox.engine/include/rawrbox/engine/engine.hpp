#pragma once

#include <rawrbox/engine/watch.hpp>
#include <rawrbox/utils/logger.hpp>

#include <atomic>

namespace rawrbox {
	enum class ENGINE_THREADS {
		NONE = 0,
		THREAD_INPUT = 1,
		THREAD_RENDER = 2
	};

	class Engine {
	protected:
		std::atomic<rawrbox::ENGINE_THREADS> _shutdown = ENGINE_THREADS::NONE;
		float _deltaTimeAccumulator = 0;

		uint32_t _tps = 66;
		uint32_t _fps = 60;

		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Engine");

		rawrbox::Watch _timer;

		// Quick sleep from https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiHelper.cpp#L1622
		virtual void sleep(float milliseconds);

		// Create the GLFW window
		virtual void setupGLFW();

		// Initialize your game
		virtual void init();
		virtual void pollEvents();
		virtual void fixedUpdate();
		virtual void update();
		virtual void draw();

		virtual void onThreadShutdown(rawrbox::ENGINE_THREADS thread);
		static void prettyPrintErr(const std::string& err);

	public:
		virtual ~Engine() = default;
		Engine() = default;

		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		virtual void shutdown();
		virtual void run();

		// sets the update rate per second
		virtual void setTPS(uint32_t ticksPerSecond);
		[[nodiscard]] virtual uint32_t getTPS() const;

		// sets the draw rate per second
		virtual void setFPS(uint32_t framesPerSecond);
		[[nodiscard]] virtual uint32_t getFPS() const;

		// returns true after quit() is called
		[[nodiscard]] bool isQuitting() const;
	};
} // namespace rawrbox
