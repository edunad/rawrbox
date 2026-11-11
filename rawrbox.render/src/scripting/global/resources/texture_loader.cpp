
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/scripting/global/resources/texture_loader.hpp>
#include <rawrbox/resources/manager.hpp>

namespace rawrbox {
	rawrbox::TextureBase* TextureLoaderGlobal::get(const std::string& path, std::optional<uint32_t> loadFlags, lua_State* L) {
		const auto resolved = rawrbox::LuaUtils::resolveContent(path, L);
		if (!resolved) throw std::runtime_error(resolved.error());

		const auto& fixedPath = resolved.value();
		if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
			fmt::print("[RawrBox-Resources] Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath.generic_string());

			const auto* ptr = rawrbox::RESOURCES::loadFile<rawrbox::ResourceTexture>(fixedPath, loadFlags.value_or(0));
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath.generic_string()));

			return ptr->get();
		}

		const auto* ptr = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(fixedPath);
		if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath.generic_string()));

		return ptr->get();
	}

	void TextureLoaderGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("texture", {})
		    .addFunction("get", &TextureLoaderGlobal::get)
		    .endNamespace();
	}

} // namespace rawrbox