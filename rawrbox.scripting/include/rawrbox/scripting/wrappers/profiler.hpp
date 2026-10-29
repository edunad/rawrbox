#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

#include <string>
#include <unordered_set>

namespace rawrbox {
	class ProfilerWrapper {
	protected:
		static std::unordered_set<std::string> _traces;

	public:
		static void zone(const std::string& name, const luabridge::LuaRef& callback);
		static void plot(const std::string& name, double value);
		static void message(const std::string& text);

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox