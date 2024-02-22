#pragma once
#include <rawrbox/scripting/utils/lua.hpp>

#include <string>
#include <unordered_map>
#include <utility>

namespace rawrbox {
	struct Hook {
		std::string name;
		luabridge::LuaRef func;

		Hook(std::string name_, luabridge::LuaRef func_) : name(std::move(name_)), func(std::move(func_)) {}
	};

	class Hooks {
	private:
		static std::unordered_map<std::string, std::vector<rawrbox::Hook>> _hooks;

	public:
		static void call(const std::string& id, const luabridge::LuaRef& func);
		static void add(const std::string& id, const std::string& name, const luabridge::LuaRef& func);
		static void remove(const std::string& id, const std::string& name);

		// Utils ---
		[[nodiscard]] static size_t count();
		[[nodiscard]] static bool empty();
		// ----

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
