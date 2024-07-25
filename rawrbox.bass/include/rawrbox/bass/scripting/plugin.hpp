#pragma once

#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/scripting/wrappers/instance.hpp>
#include <rawrbox/scripting/plugin.hpp>

#ifdef RAWRBOX_RESOURCES
	#include <rawrbox/resources/manager.hpp>
#endif

namespace rawrbox {
	class BASScripting : public rawrbox::ScriptingPlugin {
	public:
		void registerTypes(lua_State* L) override {
			rawrbox::SoundInstanceWrapper::registerLua(L);
		}

		void registerGlobal(lua_State* L) override {
			luabridge::getGlobalNamespace(L)
			    .beginNamespace("bass", {})

			    // UTILS -----
			    .addFunction("getMasterVolume", &BASS::getMasterVolume)
			    .addFunction("setMasterVolume", &BASS::setMasterVolume)
			    .addFunction("setListenerLocation", &BASS::setListenerLocation)
			    // -----

			    // LOAD -----
			    .addFunction("loadSound", [](const std::string& path, std::optional<uint32_t> flags, lua_State* L) {
				    auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");
				    auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

#ifdef RAWRBOX_RESOURCES
				    if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
					    fmt::print("Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath.generic_string());
				    }
#endif
				    auto* sound = BASS::loadSound(fixedPath, flags.value_or(0));
				    if (sound == nullptr) throw std::runtime_error(fmt::format("Failed to load sound '{}'", path));

				    return sound->createInstance();
			    })
			    .addFunction("loadHTTPSound", [](const std::string& url, std::optional<uint32_t> flags) {
				    auto* sound = BASS::loadHTTPSound(url, flags.value_or(0));
				    if (sound == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] Failed to load http sound '{}'", url));

				    return sound->createInstance();
			    })
			    // ---------

			    .endNamespace();
		}

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::BASS", "./lua/enums/bass.lua");
		}
	};
} // namespace rawrbox
