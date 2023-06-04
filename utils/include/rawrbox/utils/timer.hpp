#pragma once

#include <functional>
#include <map>
#include <string>

namespace rawrbox {
	class Timer {
	protected:
		std::string _id = "timer";
		float _nextTick = 0.F;
		float _delay = 1.F;

		int _iterations = -1;
		std::function<void()> _func = nullptr;
		std::function<void()> _onComplete = nullptr;

		bool _paused = false;
		bool _infinite = false;

		// Internal
		float _pausedTime = 0.F;
		// -----

	public:
		static uint32_t ID;
		static std::unordered_map<std::string, rawrbox::Timer> timers;

		// STATIC ----
		static void update();
		static Timer simple(int msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);
		static Timer create(int reps, int msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);
		static const bool isRunning(const std::string& id);
		static void clear();
		// ----

		void stop();
		void start();
		void setPaused(bool pause);
	};
} // namespace rawrbox
