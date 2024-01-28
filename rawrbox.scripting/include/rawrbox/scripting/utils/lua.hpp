#pragma once

// -------------------------------
#include <lualib.h>
// --- LuaBridge.h needs to be included after lualib.h!
#include <LuaBridge/LuaBridge.h>
// -------------------------------

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>

namespace rawrbox {
	class LuaUtils {
	public:
		static void compileAndLoad(lua_State* L, const std::string& chunkID, const std::filesystem::path& path, bool run = true);
		static void resume(lua_State* L, lua_State* from);
		static void run(lua_State* L);
		static void collect_garbage(lua_State* L);

		static std::string getError(lua_State* L);

		static luabridge::LuaRef jsonToLua(lua_State* L, const nlohmann::json& json);
		static nlohmann::json luaToJsonObject(lua_State* L);

		template <typename... CallbackArgs>
		static void runCallback(const luabridge::LuaRef& func, CallbackArgs&&... args) {
			if (!func.isCallable()) return;
			fnc(func, std::forward<CallbackArgs>(args)...);
		}

		// #/ == System content
		// @/ == Root content
		// @cats/ == `cats` mod
		// normal_path == current mod
		static std::string getContent(const std::filesystem::path& path, const std::filesystem::path& modPath);
	};
} // namespace rawrbox
