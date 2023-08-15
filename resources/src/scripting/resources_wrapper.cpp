
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/resources/scripting/resources_wrapper.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {

	ResourcesWrapper::ResourcesWrapper(rawrbox::Mod* mod) : _mod(mod) {}

	void ResourcesWrapper::preLoadFolder(const std::string& path) {
		if (this->_mod == nullptr) throw std::runtime_error("[RawrBox-Resources] MOD not set!");

		auto fixedPath = rawrbox::LuaUtils::getContent(path, this->_mod->getFolder().generic_string());
		rawrbox::RESOURCES::preLoadFolder(fixedPath);
	}

	void ResourcesWrapper::preLoad(const std::string& path, sol::optional<uint32_t> loadFlags) {
		if (this->_mod == nullptr) throw std::runtime_error("[RawrBox-Resources] MOD not set!");

		auto fixedPath = rawrbox::LuaUtils::getContent(path, this->_mod->getFolder().generic_string());
		rawrbox::RESOURCES::preLoadFile(fixedPath, loadFlags.value_or(0));
	}

	std::string ResourcesWrapper::getContent(sol::optional<std::string> path) {
		return rawrbox::LuaUtils::getContent(path.value_or(""), this->_mod->getFolder().generic_string());
	}

	void ResourcesWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ResourcesWrapper>("resources",
		    sol::no_constructor,
		    "preLoad", &ResourcesWrapper::preLoad,
		    "preLoadFolder", &ResourcesWrapper::preLoadFolder,

		    "getContent", &ResourcesWrapper::getContent);
	}

} // namespace rawrbox
