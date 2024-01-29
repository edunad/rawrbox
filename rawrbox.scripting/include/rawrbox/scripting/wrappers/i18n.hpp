#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class I18NWrapper {
	public:
		[[nodiscard]] static const std::string& getLanguage();
		static void setLanguage(const std::string& language);

		static std::string get(const luabridge::LuaRef& ref);

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
