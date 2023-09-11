
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/scripting/wrappers/resources/font_loader_wrapper.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	rawrbox::FontWrapper FontLoaderWrapper::get(const std::string& path, sol::optional<uint16_t> size, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-FontWrapper] MOD not set!");

		std::string modFolder = modEnv.env.value()["__mod_folder"];
		auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

		if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
			fmt::print("[RawrBox-Resources] Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath);

			auto ptr = rawrbox::RESOURCES::loadFile<rawrbox::ResourceFont>(fixedPath, 0);
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath));

			return {ptr->getSize(size.value_or(12))};
		} else {
			auto ptr = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>(fixedPath);
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Resources] '{}' not found!", fixedPath));

			return {ptr->getSize(size.value_or(12))};
		}
	}

	void FontLoaderWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<FontLoaderWrapper>("fontLoader",
		    sol::no_constructor,
		    "get", &FontLoaderWrapper::get);
	}

} // namespace rawrbox
