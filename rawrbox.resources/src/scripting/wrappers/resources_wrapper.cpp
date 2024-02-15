
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/resources/scripting/wrappers/resources_wrapper.hpp>

namespace rawrbox {
	void ResourcesWrapper::preLoadFolder(const std::string& path, lua_State* L) {
		if (L == nullptr) throw std::runtime_error("Lua env not set");

		auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

		rawrbox::RESOURCES::preLoadFolder(fixedPath);
	}

	void ResourcesWrapper::preLoad(const std::string& path, std::optional<uint32_t> loadFlags, lua_State* L) {
		if (L == nullptr) throw std::runtime_error("Lua env not set");

		auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

		rawrbox::RESOURCES::preLoadFile(fixedPath, loadFlags.value_or(0));
	}

	std::string ResourcesWrapper::getContent(std::optional<std::string> path, lua_State* L) {
		if (L == nullptr) throw std::runtime_error("Lua env not set");

		auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
		return rawrbox::LuaUtils::getContent(path.value_or(""), modFolder);
	}

	void ResourcesWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("resources", {})
		    .addFunction("preLoad", &rawrbox::ResourcesWrapper::preLoad)
		    .addFunction("preLoadFolder", &rawrbox::ResourcesWrapper::preLoadFolder)
		    .addFunction("getContent", &rawrbox::ResourcesWrapper::getContent)
		    .endNamespace();
	}

} // namespace rawrbox
