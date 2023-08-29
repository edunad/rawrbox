
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/resources/scripting/wrappers/resources_wrapper.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	void ResourcesWrapper::preLoadFolder(const std::string& path, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-ResourcesWrapper] MOD not set!");
		std::string modFolder = modEnv.env.value()["__mod_folder"];

		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);
		rawrbox::RESOURCES::preLoadFolder(fixedPath);
	}

	void ResourcesWrapper::preLoad(const std::string& path, sol::optional<uint32_t> loadFlags, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-ResourcesWrapper] MOD not set!");
		std::string modFolder = modEnv.env.value()["__mod_folder"];

		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);
		rawrbox::RESOURCES::preLoadFile(fixedPath, loadFlags.value_or(0));
	}

	std::string ResourcesWrapper::getContent(sol::optional<std::string> path, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-ResourcesWrapper] MOD not set!");

		std::string modFolder = modEnv.env.value()["__mod_folder"];
		return rawrbox::LuaUtils::getContent(path.value_or(""), modFolder);
	}

	void ResourcesWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ResourcesWrapper>("resources",
		    sol::no_constructor,
		    "preLoad", &ResourcesWrapper::preLoad,
		    "preLoadFolder", &ResourcesWrapper::preLoadFolder,

		    "getContent", &ResourcesWrapper::getContent);
	}

} // namespace rawrbox
