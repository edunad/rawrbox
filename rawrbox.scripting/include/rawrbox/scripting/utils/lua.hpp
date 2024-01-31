#pragma once

// -------------------------------
#include <lualib.h>
// --- LuaBridge.h needs to be included after lualib.h!
#include <LuaBridge/LuaBridge.h>
// -------------------------------

#include <rawrbox/utils/logger.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace rawrbox {
	class LuaUtils {
	public:
		static void compileAndLoad(lua_State* L, const std::string& chunkID, const std::filesystem::path& path);
		static void resume(lua_State* L, lua_State* from);
		static void run(lua_State* L);
		static void collect_garbage(lua_State* L);

		static std::string getError(lua_State* L);

		static std::vector<std::string> getStringVariadicArgs(lua_State* L);
		static void getVariadicArgs(const luabridge::LuaRef& in, luabridge::LuaRef& out);

		static luabridge::LuaRef jsonToLua(lua_State* L, const nlohmann::json& json);
		static nlohmann::json luaToJsonObject(lua_State* L);

		static std::string getLuaENVVar(lua_State* L, const std::string& varId);

		// #/ == System content
		// @/ == Root content
		// @cats/ == `cats` mod
		// normal_path == current mod
		static std::string getContent(const std::filesystem::path& path, const std::filesystem::path& modPath);
	};
} // namespace rawrbox
