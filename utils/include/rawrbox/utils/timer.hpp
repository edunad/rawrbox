#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace rawrbox {
	class Timer {
	protected:
		uint32_t _id = 0;

		uint64_t _nextTick = 0;
		uint64_t _msDelay = 0;
		uint64_t _pausedTime = 0;

		int _iterations = -1;
		int _ticks = 0;

		std::function<void()> _func = nullptr;
		std::function<void()> _onComplete = nullptr;

		bool _paused = false;
		bool _infinite = false;

	public:
		static uint32_t ID;
		static std::unordered_map<uint32_t, std::unique_ptr<rawrbox::Timer>> timers;

		// STATIC ----
		static void update();
		static rawrbox::Timer* simple(uint64_t msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);
		static rawrbox::Timer* create(int reps, uint64_t msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);
		static bool isRunning(uint32_t id);
		static void clear();
		// ----

		void stop();
		void start();
		void setPaused(bool pause);
	};
} // namespace rawrbox
