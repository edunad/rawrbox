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
#include <vector>

namespace rawrbox {
	// New luau & luabridge removed lua errors from calls
	class LuaResult {
	protected:
		std::vector<luabridge::LuaRef> _values = {};
		std::string _error;

	public:
		LuaResult(std::vector<luabridge::LuaRef> values, std::string error);

		[[nodiscard]] bool wasOk() const;
		[[nodiscard]] bool hasFailed() const;
		[[nodiscard]] const std::string& errorMessage() const;

		[[nodiscard]] size_t size() const;
		[[nodiscard]] const luabridge::LuaRef& operator[](size_t index) const;
	};

	class LuaUtils {
	public:
		template <typename... CallbackArgs>
		static rawrbox::LuaResult call(const luabridge::LuaRef& func, CallbackArgs&&... args) {
			lua_State* L = func.state();
			if (L == nullptr) return {{}, "Invalid lua state"};

			const int top = lua_gettop(L);
			func.push(L);

			// ARGS ---
			const bool pushed = (true && ... && luabridge::push(L, std::forward<CallbackArgs>(args)));

			if (!pushed) {
				lua_settop(L, top);
				return {{}, "Failed to push lua function arguments"};
			}
			// ----

			if (lua_pcall(L, static_cast<int>(sizeof...(CallbackArgs)), LUA_MULTRET, 0) != LUA_OK) {
				std::string error = getError(L);
				lua_settop(L, top);

				return {{}, error};
			}

			// Results ---
			std::vector<luabridge::LuaRef> values = {};
			for (int i = top + 1; i <= lua_gettop(L); i++) {
				values.push_back(luabridge::LuaRef::fromStack(L, i));
			}

			lua_settop(L, top);
			return {std::move(values), ""};
			// ----
		}

		static void compileAndLoadFile(lua_State* L, const std::string& chunkID, const std::filesystem::path& path);
		static void compileAndLoadScript(lua_State* L, const std::string& chunkID, const std::string& script);

		static void resume(lua_State* L, lua_State* from);
		static void run(lua_State* L);
		static void collect_garbage(lua_State* L);

		static std::string getError(lua_State* L);

		static std::vector<std::string> argsToString(lua_State* L, bool filterNonStr = false);
		static void getVariadicArgs(const luabridge::LuaRef& in, luabridge::LuaRef& out);

		static luabridge::LuaRef jsonToLua(lua_State* L, const glz::generic& json);
		static glz::generic luaToJsonObject(const luabridge::LuaRef& ref);

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
