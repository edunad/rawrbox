#pragma once

#include <rawrbox/resources/manager.hpp>
#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

#include <cmath>
#include <limits>

namespace rawrbox {
	class ResourcesPlugin : public rawrbox::ScriptingPlugin {
	public:
		void registerGlobal(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Invalid lua state");

			luabridge::getGlobalNamespace(L)
			    .beginNamespace("resources", {})
			    .addFunction("preLoadFolder", [](const std::string& path, lua_State* L) {
				    if (L == nullptr) throw std::runtime_error("Lua env not set");

				    const auto fixedPath = rawrbox::LuaUtils::resolveContent(path, L);
				    if (!fixedPath) throw std::runtime_error(fixedPath.error());

				    rawrbox::RESOURCES::preLoadFolder(fixedPath.value());
			    })
			    .addFunction("preLoad", [](const std::string& path, std::optional<double> loadFlags, lua_State* L) {
				    if (L == nullptr) throw std::runtime_error("Lua env not set");

				    const double flags = loadFlags.value_or(0.0);
				    if (flags < 0.0 || flags > static_cast<double>(std::numeric_limits<uint32_t>::max()))
					    throw std::runtime_error(fmt::format("Invalid load flags '{}'", flags));

				    const auto fixedPath = rawrbox::LuaUtils::resolveContent(path, L);
				    if (!fixedPath) throw std::runtime_error(fixedPath.error());

				    rawrbox::RESOURCES::preLoadFile(fixedPath.value(), static_cast<uint32_t>(flags));
			    })
			    .addFunction("exists", [](const std::string& path, lua_State* L) {
				    if (L == nullptr) throw std::runtime_error("Lua env not set");

				    const auto fixedPath = rawrbox::LuaUtils::resolveContent(path, L);
				    return fixedPath.has_value() && rawrbox::VFS::contains(fixedPath.value());
			    })
			    .addFunction("getContent", [](const std::optional<std::string>& path, lua_State* L) {
				    if (L == nullptr) throw std::runtime_error("Lua env not set");

				    const auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");
				    if (modFolder.empty()) throw std::runtime_error("Mod folder not set in Lua env");

				    return rawrbox::LuaUtils::getContent(path.value_or(""), modFolder);
			    })
			    .endNamespace();
		}

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::RESOURCES", "./lua/enums/resources.lua");
		}
	};
} // namespace rawrbox