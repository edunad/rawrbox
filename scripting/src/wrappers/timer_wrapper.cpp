#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/timer_wrapper.hpp>
#include <rawrbox/utils/timer.hpp>

namespace rawrbox {
	// CREATE ---
	bool TimerWrapper::create(const std::string& id, int reps, uint64_t delay, sol::function callback, sol::function onComplete) {
		auto timer = rawrbox::TIMER::create(
		    id, reps, delay, [callback]() { rawrbox::LuaUtils::runCallback(callback); },
		    [onComplete]() {
			    rawrbox::LuaUtils::runCallback(onComplete);
		    });

		return timer != nullptr;
	}

	bool TimerWrapper::simple(const std::string& id, uint64_t delay, sol::function callback, sol::function onComplete) {
		auto timer = rawrbox::TIMER::simple(
		    id, delay, [callback]() { rawrbox::LuaUtils::runCallback(callback); },
		    [onComplete]() {
			    rawrbox::LuaUtils::runCallback(onComplete);
		    });

		return timer != nullptr;
	}
	// ----

	// UTILS ---
	bool TimerWrapper::destroy(const std::string& id) {
		return rawrbox::TIMER::destroy(id);
	}

	bool TimerWrapper::exists(const std::string& id) {
		return rawrbox::TIMER::exists(id);
	}

	bool TimerWrapper::pause(const std::string& id, bool pause) {
		return rawrbox::TIMER::pause(id, pause);
	}
	// ----

	void TimerWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::TimerWrapper>("TIMER",
		    sol::no_constructor,
		    // CREATE -----
		    "create", &TimerWrapper::create,
		    "simple", &TimerWrapper::simple,
		    // ----
		    "destroy", &TimerWrapper::destroy,
		    "exists", &TimerWrapper::exists,
		    "pause", &TimerWrapper::pause);
	}
} // namespace rawrbox
