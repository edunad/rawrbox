#pragma once

#include <nlohmann/json.hpp>

#include <lualib.h>
//----
#include <LuaBridge/LuaBridge.h>

#include <filesystem>
#include <string>

namespace rawrbox {
	class LuaUtils {
	public:
		static void compileAndLoad(lua_State* L, const std::string& chunkID, const std::filesystem::path& path);
		static std::string getError(lua_State* L);

		static nlohmann::json luaToJsonObject(lua_State* L);
		static nlohmann::json luaToJsonObject(const luabridge::LuaRef& ref);
	};
} // namespace rawrbox
