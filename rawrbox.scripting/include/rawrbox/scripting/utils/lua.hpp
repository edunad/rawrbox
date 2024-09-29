#pragma once

#pragma warning(push)
#pragma warning(disable : 4702)
#include <lualib.h>
// --- LuaBridge.h needs to be included after lualib.h!
#include <LuaBridge/LuaBridge.h>
#pragma warning(pop)

#include <rawrbox/utils/logger.hpp>

#include <glaze/glaze.hpp>

#include <Luau/Compiler.h>

#include <filesystem>
#include <string>

namespace DFInt {
	extern int LuauTypeSolverRelease;
	extern int LuauSolverV2;
} // namespace DFInt

namespace rawrbox {
	class LuaUtils {
	public:
		static void compileAndLoadFile(lua_State* L, const std::string& chunkID, const std::filesystem::path& path);
		static void compileAndLoadScript(lua_State* L, const std::string& chunkID, const std::string& script);

		static void resume(lua_State* L, lua_State* from);
		static void run(lua_State* L);
		static void collect_garbage(lua_State* L);

		static std::string getError(lua_State* L);

		static std::vector<std::string> argsToString(lua_State* L, bool filterNonStr = false);
		static void getVariadicArgs(const luabridge::LuaRef& in, luabridge::LuaRef& out);

		static luabridge::LuaRef jsonToLua(lua_State* L, const glz::json_t& json);
		static glz::json_t luaToJsonObject(const luabridge::LuaRef& ref);

		template <typename T>
			requires(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
		static T getLuaENVVar(lua_State* L, const std::string& varId) {
			if (L == nullptr) throw std::runtime_error("Invalid lua state");

			lua_getfield(L, LUA_ENVIRONINDEX, varId.c_str());

			const auto* conv = lua_tostring(L, -1);
			if (conv == nullptr) throw std::runtime_error(fmt::format("Invalid lua env variable '{}'", varId));
			if constexpr (std::is_same_v<T, std::string>) {
				return conv;
			} else {
				return static_cast<T>(conv);
			}
		}

		template <typename T>
			requires(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>)
		static luabridge::LuaRef vectorToTable(lua_State* L, const std::vector<T>& in) {
			auto tbl = luabridge::newTable(L);
			for (size_t i = 0; i < in.size(); i++) {
				tbl[i + 1] = in[i];
			}

			return tbl;
		}

		// #/ == Root content
		// normal_path == current mod
		// @cats/ == `cats` mod
		static std::pair<std::string, std::filesystem::path> getContent(const std::filesystem::path& path, const std::filesystem::path& modPath);
	};
} // namespace rawrbox
