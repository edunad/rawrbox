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

		static luabridge::LuaRef jsonToLua(lua_State* L, const nlohmann::json& json);
		static nlohmann::json luaToJsonObject(lua_State* L);
	};
} // namespace rawrbox
