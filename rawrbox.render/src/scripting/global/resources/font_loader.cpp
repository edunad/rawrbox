
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/scripting/global/resources/font_loader.hpp>
#include <rawrbox/resources/manager.hpp>

namespace rawrbox {
	rawrbox::Font* FontLoaderGlobal::get(const std::string& path, std::optional<uint16_t> size, lua_State* L) {
		auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");
		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

		if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
			fmt::print("[RawrBox-Resources] Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath);

			auto* ptr = rawrbox::RESOURCES::loadFile<rawrbox::ResourceFont>(fixedPath, 0);
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath));

			return ptr->getSize(size.value_or(12));
		}

		auto* ptr = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>(fixedPath);
		if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath));

		return ptr->getSize(size.value_or(12));
	}

	void FontLoaderGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("font", {})
		    .addFunction("get", &FontLoaderGlobal::get)
		    .endNamespace();
	}

} // namespace rawrbox
