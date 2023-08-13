#pragma once

#include <rawrbox/scripting/scripting.hpp>

namespace rawrbox {
	class TimerWrapper {
	public:
		TimerWrapper() = default;
		TimerWrapper(const TimerWrapper&) = default;
		TimerWrapper(TimerWrapper&&) = default;
		TimerWrapper& operator=(const TimerWrapper&) = default;
		TimerWrapper& operator=(TimerWrapper&&) = default;
		virtual ~TimerWrapper() = default;

		// CREATE ---
		virtual bool create(const std::string& id, int reps, uint64_t delay, sol::function callback, sol::function onComplete);
		virtual bool simple(const std::string& id, uint64_t delay, sol::function callback, sol::function onComplete);
		// ----

		// UTILS ---
		virtual bool destroy(const std::string& id);
		virtual bool exists(const std::string& id);
		virtual bool pause(const std::string& id, bool pause);
		// ----

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
