
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/scripting/global/resources/texture_loader.hpp>
#include <rawrbox/resources/manager.hpp>

namespace rawrbox {
	rawrbox::TextureBase* TextureLoaderGlobal::get(const std::string& path, std::optional<uint32_t> loadFlags, lua_State* L) {
		auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");

		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);
		if (!fixedPath.first.empty()) throw std::runtime_error("External mod resources loading not supported");

		if (!rawrbox::RESOURCES::isLoaded(fixedPath.second)) {
			fmt::print("[RawrBox-Resources] Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath.second.generic_string());

			auto* ptr = rawrbox::RESOURCES::loadFile<rawrbox::ResourceTexture>(fixedPath.second, loadFlags.value_or(0));
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath.second.generic_string()));

			return ptr->get();
		}

		auto* ptr = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(fixedPath.second);
		if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath.second.generic_string()));

		return ptr->get();
	}

	void TextureLoaderGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("texture", {})
		    .addFunction("get", &TextureLoaderGlobal::get)
		    .endNamespace();
	}

} // namespace rawrbox
