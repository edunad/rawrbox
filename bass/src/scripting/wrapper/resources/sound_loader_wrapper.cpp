
#include <rawrbox/bass/resources/sound.hpp>
#include <rawrbox/bass/scripting/wrapper/resources/sound_loader_wrapper.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

// TODO: remove bass loader
namespace rawrbox {
	SoundLoaderWrapper::SoundLoaderWrapper(rawrbox::Mod* mod) : _mod(mod) {}

	rawrbox::SoundInstanceWrapper SoundLoaderWrapper::get(const std::string& path, sol::optional<uint32_t> loadFlags) {
		if (this->_mod == nullptr) throw std::runtime_error("[RawrBox-BASSLoader] MOD not set!");
		auto fixedPath = rawrbox::LuaUtils::getContent(path, this->_mod->getFolder().generic_string());

		if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
			fmt::print("[RawrBox-BASSLoader] Loading '{}' RUNTIME! You should load content on the 'MOD:onLoad' stage instead!", fixedPath);

			auto ptr = rawrbox::RESOURCES::loadFile<rawrbox::SoundBase>(fixedPath, loadFlags.value_or(0));
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-BASSLoader] '{}' not found!", fixedPath));

			return {ptr->createInstance()};
		} else {
			auto ptr = rawrbox::RESOURCES::getFile<rawrbox::SoundBase>(fixedPath);
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-BASSLoader] '{}' not found!", fixedPath));

			return {ptr->createInstance()};
		}
	}

	void SoundLoaderWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<SoundLoaderWrapper>("soundLoader",
		    sol::no_constructor,
		    "get", &SoundLoaderWrapper::get);
	}

} // namespace rawrbox
