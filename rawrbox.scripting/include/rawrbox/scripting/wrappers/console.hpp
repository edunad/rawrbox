#pragma once

#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/console.hpp>

namespace rawrbox {
	class ConsoleWrapper {
		static rawrbox::Console* _console;

	public:
		static void init(rawrbox::Console* console);
		static void shutdown();

		// UTILS ---
		static void clear();
		static const rawrbox::ConsoleCommand& get(const std::string& command);
		// ---------

		static std::pair<bool, std::string> execute(lua_State* L);
		static void print(const std::string& text, std::optional<rawrbox::PrintType> type);

		static bool registerMethod(const std::string& command, const luabridge::LuaRef& callback, const std::optional<std::string>& description, std::optional<uint32_t> flags);

		static bool removeCommand(const std::string& command);
		static bool hasCommand(const std::string& command);
		// static bool registerVariable(const std::string& command, const luabridge::LuaRef& callback, std::optional<std::string> description, std::optional<uint32_t> flags);

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
