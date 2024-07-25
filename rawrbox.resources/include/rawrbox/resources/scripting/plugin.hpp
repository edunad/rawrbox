#pragma once

#include <rawrbox/resources/manager.hpp>
#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class ResourcesPlugin : public rawrbox::ScriptingPlugin {
	public:
		void registerGlobal(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Invalid lua state");

			luabridge::getGlobalNamespace(L)
			    .beginNamespace("resources", {})
			    .addFunction("preLoadFolder", [](const std::string& path, lua_State* L) {
				    if (L == nullptr) throw std::runtime_error("Lua env not set");

				    auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");
				    if (modFolder.empty()) throw std::runtime_error("Mod folder not set in Lua env");

				    rawrbox::RESOURCES::preLoadFolder(rawrbox::LuaUtils::getContent(path, modFolder));
			    })
			    .addFunction("preLoad", [](const std::string& path, std::optional<uint32_t> loadFlags, lua_State* L) {
				    if (L == nullptr) throw std::runtime_error("Lua env not set");

				    auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");
				    if (modFolder.empty()) throw std::runtime_error("Mod folder not set in Lua env");

				    rawrbox::RESOURCES::preLoadFile(rawrbox::LuaUtils::getContent(path, modFolder), loadFlags.value_or(0));
			    })
			    .addFunction("getContent", [](const std::optional<std::string>& path, lua_State* L) {
				    if (L == nullptr) throw std::runtime_error("Lua env not set");

				    auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");
				    if (modFolder.empty()) throw std::runtime_error("Mod folder not set in Lua env");

				    return rawrbox::LuaUtils::getContent(path.value_or(""), modFolder);
			    })
			    .endNamespace();
		}
	};
} // namespace rawrbox
