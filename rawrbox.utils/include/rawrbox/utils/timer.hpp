#pragma once

#include <functional>
#include <memory>
#include <string>

namespace rawrbox {
	class TIMER {
	protected:
		std::string _id;

		float _nextTick = 0;
		float _msDelay = 0;
		float _pausedTime = 0;

		int _iterations = -1;
		int _ticks = 0;

		std::function<void()> _func = nullptr;
		std::function<void()> _onComplete = nullptr;

		bool _paused = false;
		bool _infinite = false;

		static rawrbox::TIMER* init(const std::string& id, int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);

	public:
		static uint32_t ID;
		static std::unordered_map<std::string, std::unique_ptr<rawrbox::TIMER>> timers;

		// STATIC ----
		static void update();

		static rawrbox::TIMER* simple(const std::string& id, float msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);
		static rawrbox::TIMER* simple(float msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);
		static rawrbox::TIMER* create(const std::string& id, int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);
		static rawrbox::TIMER* create(int reps, float msDelay, std::function<void()> func, std::function<void()> onComplete = nullptr);

		static bool destroy(const std::string& id);
		static bool pause(const std::string& id, bool pause);
		static bool exists(const std::string& id);
		static void clear();
		// ----

		void destroy();
		void pause(bool pause);
	};
} // namespace rawrbox
