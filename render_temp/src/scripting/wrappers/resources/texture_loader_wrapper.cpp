
#include <rawrbox/render_temp/resources/texture.hpp>
#include <rawrbox/render_temp/scripting/wrappers/resources/texture_loader_wrapper.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	rawrbox::TextureWrapper TextureLoaderWrapper::get(const std::string& path, sol::optional<uint32_t> loadFlags, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-TextureWrapper] MOD not set!");

		std::string modFolder = modEnv.env.value()["__mod_folder"];
		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

		if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
			fmt::print("[RawrBox-Resources] Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath);

			auto ptr = rawrbox::RESOURCES::loadFile<rawrbox::ResourceTexture>(fixedPath, loadFlags.value_or(0));
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath));

			return {ptr->get()};
		} else {
			auto ptr = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(fixedPath);
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath));

			return {ptr->get()};
		}
	}

	void TextureLoaderWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<TextureLoaderWrapper>("textureLoader",
		    sol::no_constructor,
		    "get", &TextureLoaderWrapper::get);
	}

} // namespace rawrbox
