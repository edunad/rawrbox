
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/scripting/global/resources/texture_loader.hpp>
#include <rawrbox/resources/manager.hpp>

namespace rawrbox {
	rawrbox::TextureBase* TextureLoaderGlobal::get(const std::string& path, std::optional<uint32_t> loadFlags, lua_State* L) {
		auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

		if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
			fmt::print("[RawrBox-Resources] Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath);

			auto* ptr = rawrbox::RESOURCES::loadFile<rawrbox::ResourceTexture>(fixedPath, loadFlags.value_or(0));
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath));

			return ptr->get();
		}

		auto* ptr = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(fixedPath);
		if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath));

		return ptr->get();
	}

	void TextureLoaderGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("texture", {})
		    .addFunction("get", &TextureLoaderGlobal::get)
		    .endNamespace();
	}

} // namespace rawrbox
