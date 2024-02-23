#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class TimerWrapper {
	public:
		// CREATE ---
		static bool create(const std::string& id, int reps, float delay, const luabridge::LuaRef& callback, std::optional<luabridge::LuaRef> onComplete);
		static bool simple(const std::string& id, float delay, const luabridge::LuaRef& callback, std::optional<luabridge::LuaRef> onComplete);
		// ---

		// UTILS ---
		static bool destroy(const std::string& id);
		static bool exists(const std::string& id);
		static bool pause(const std::string& id, bool pause);
		// ----

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
