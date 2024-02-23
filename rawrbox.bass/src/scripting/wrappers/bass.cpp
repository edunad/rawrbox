#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/scripting/wrappers/bass.hpp>

#ifdef RAWRBOX_RESOURCES
	#include <rawrbox/resources/manager.hpp>
#endif

namespace rawrbox {
	void BASSWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("bass", {})
		    // UTILS -----
		    .addFunction("getMasterVolume", &BASS::getMasterVolume)
		    .addFunction("setMasterVolume", &BASS::setMasterVolume)
		    .addFunction("setListenerLocation", &BASS::setListenerLocation)
		    // -----

		    // LOAD -----
		    .addFunction("loadSound", [](const std::string& path, std::optional<uint32_t> flags, lua_State* L) {
			    auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
			    auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

#ifdef RAWRBOX_RESOURCES
			    if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
				    fmt::print("Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath);
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
} // namespace rawrbox
