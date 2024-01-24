#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/path.hpp>

#include <Luau/Compiler.h>
#include <fmt/format.h>

namespace rawrbox {

	void LuaUtils::compileAndLoad(lua_State* L, const std::string& chunkID, const std::filesystem::path& path) {
		// Load script ---
		auto bytes = rawrbox::PathUtils::getRawData(path);
		if (bytes.empty()) throw std::runtime_error("File empty / failed to load");
		// --------------

		// Compile ----
		Luau::CompileOptions options = {};
#ifndef _DEBUG
		options.optimizationLevel = 2; // Remove debug info & inline lua
		options.debugLevel = 0;
#endif

		Luau::ParseOptions parser = {};
		parser.allowDeclarationSyntax = true; // ?
		parser.captureComments = false;

		std::string bytecode = "";

		try {
			bytecode = Luau::compile(std::string(bytes.begin(), bytes.end()), options, parser);
			if (bytecode.empty()) throw std::runtime_error("Failed to compile");
		} catch (std::exception& error) {
			throw std::runtime_error(error.what());
		}
		// ----------

		// Load -------
		try {
			std::string chunk = fmt::format("={}", chunkID);
			if (luau_load(L, chunk.c_str(), bytecode.data(), bytecode.size(), 0) != 0) {
				throw std::runtime_error(rawrbox::LuaUtils::getError(L));
			}
		} catch (std::exception& error) {
			throw std::runtime_error(error.what());
		}
		// -----------
	}

	std::string LuaUtils::getError(lua_State* L) {
		const char* error_message = lua_tostring(L, -1);
		lua_pop(L, 1); // Remove error

		return error_message;
	}

	luabridge::LuaRef LuaUtils::jsonToLua(lua_State* L, const nlohmann::json& json) {
		if (json.is_null()) {
			return {L, luabridge::LuaNil()};
		} else if (json.is_boolean()) {
			return {L, json.get<bool>()};
		} else if (json.is_number()) { // Lua only works with doubles
			return {L, json.get<double>()};
		} else if (json.is_string()) {
			return {L, json.get<std::string>()};
		} else if (json.is_object()) {
			auto obj = luabridge::newTable(L);
			for (nlohmann::json::const_iterator it = json.begin(); it != json.end(); ++it) {
				obj[it.key().c_str()] = jsonToLua(L, *it);
			}

			return obj;
		} else if (json.is_array()) {
			auto obj = luabridge::newTable(L);
			unsigned long i = 1;
			for (const auto& it : json) {
				obj[i++] = jsonToLua(L, it);
			}

			return obj;
		}

		throw std::runtime_error("Unknown json type");
	}

	// https://github.com/Henningstone/HMod/blob/3061f74e6e8f7b81a91bb2980725b44daf9c8c23/src/engine/server/lua/luajson.cpp#L173
	nlohmann::json LuaUtils::luaToJsonObject(lua_State* L) {
		nlohmann::json result = {};

		if (lua_type(L, -1) != LUA_TTABLE) return result; // Not a table? meh
		lua_pushnil(L);

		int indx = 1; // Lua starts at 1
		while (lua_next(L, -2) != 0) {
			auto type = lua_type(L, -1);
			std::string key = lua_type(L, -2) == LUA_TNUMBER ? std::to_string(indx++) : lua_tostring(L, -2); // Vectors don't have a key

			switch (type) {
				case LUA_TBOOLEAN:
					result[key] = lua_toboolean(L, -1) != 0;
					break;
				case LUA_TNUMBER:
					result[key] = lua_tonumber(L, -1);
					break;
				case LUA_TSTRING:
					result[key] = lua_tostring(L, -1);
					break;
				case LUA_TVECTOR: // eeehhh, might die?
				case LUA_TTABLE:
					result[key] = luaToJsonObject(L);
					break;
				case LUA_TFUNCTION:
					result[key] = fmt::format("function({})", lua_topointer(L, -1));
					break;
				default:
				case LUA_TNONE:
				case LUA_TNIL:
					break;
			}

			lua_pop(L, 1);
		}

		return result;
	}
} // namespace rawrbox
