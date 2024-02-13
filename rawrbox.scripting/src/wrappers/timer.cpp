#include <rawrbox/scripting/wrappers/timer.hpp>
#include <rawrbox/utils/timer.hpp>

namespace rawrbox {
	// CREATE ---
	bool TimerWrapper::create(const std::string& id, int reps, float delay, const luabridge::LuaRef& callback, std::optional<luabridge::LuaRef> onComplete) {
		if (!callback.isCallable()) throw std::runtime_error("Invalid callback");
		if (onComplete.has_value() && !onComplete->isCallable()) throw std::runtime_error("Invalid onComplete callback");

		auto timer = rawrbox::TIMER::create(
		    id, reps, delay, [callback]() {
				auto result = luabridge::call(callback);
				if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage()); },
		    [onComplete]() {
			    if (!onComplete.has_value()) return;
			    auto result = luabridge::call(onComplete.value());
			    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
		    });

		return timer != nullptr;
	}

	bool TimerWrapper::simple(const std::string& id, float delay, const luabridge::LuaRef& callback, std::optional<luabridge::LuaRef> onComplete) {
		if (!callback.isCallable()) throw std::runtime_error("Invalid callback");
		if (onComplete.has_value() && !onComplete->isCallable()) throw std::runtime_error("Invalid onComplete callback");

		auto timer = rawrbox::TIMER::simple(
		    id, delay, [callback]() {
				auto result = luabridge::call(callback);
			    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage()); },
		    [onComplete]() {
			    if (!onComplete.has_value()) return;
			    auto result = luabridge::call(onComplete.value());
			    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
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

	void TimerWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("timer", {})
		    // CREATE -----
		    .addFunction("create", &rawrbox::TimerWrapper::create)
		    .addFunction("simple", &rawrbox::TimerWrapper::simple)
		    // ----
		    .addFunction("destroy", &rawrbox::TimerWrapper::destroy)
		    .addFunction("exists", &rawrbox::TimerWrapper::exists)
		    .addFunction("pause", &rawrbox::TimerWrapper::pause)
		    .endNamespace();
	}
} // namespace rawrbox
