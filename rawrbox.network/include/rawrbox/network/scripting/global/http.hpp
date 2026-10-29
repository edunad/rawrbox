#pragma once

#include <rawrbox/network/http.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

#include <unordered_map>

namespace rawrbox {
	class HTTPGlobal {
	protected:
		static std::unordered_map<uint64_t, luabridge::LuaRef> _callbacks; // Render thread only
		static uint64_t _id;

	public:
		// UTILS -----
		static void request(const std::string& url, int method, const luabridge::LuaRef& headers, const luabridge::LuaRef& callback, std::optional<int> timeout);
		// ----------------

		static void clear(rawrbox::Mod& mod);
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
