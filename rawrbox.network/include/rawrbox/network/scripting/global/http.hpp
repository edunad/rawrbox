#pragma once

#include <rawrbox/network/http.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class HTTPGlobal {
	public:
		// UTILS -----
		static void request(const std::string& url, int method, const luabridge::LuaRef& headers, const luabridge::LuaRef& callback, std::optional<int> timeout);
		// ----------------

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
