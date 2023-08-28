
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/scripting/wrappers/resources/font_loader_wrapper.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	FontLoaderWrapper::FontLoaderWrapper(rawrbox::Mod* mod) : _mod(mod) {}

	rawrbox::FontWrapper FontLoaderWrapper::get(const std::string& path, sol::optional<uint32_t> size) {
		if (this->_mod == nullptr) throw std::runtime_error("[RawrBox-FontLoader] MOD not set!");
		auto fixedPath = rawrbox::LuaUtils::getContent(path, this->_mod->getFolder());

		if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
			fmt::print("[Resources] Loading '{}' RUNTIME! You should load content on the mod's load stage!\n", fixedPath);

			auto ptr = rawrbox::RESOURCES::loadFile<rawrbox::ResourceFont>(fixedPath, 0);
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[Resources] '{}' not found!", fixedPath));

			return {ptr->getSize(size.value_or(12))};
		} else {
			auto ptr = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>(fixedPath);
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[Resources] '{}' not found!", fixedPath));

			return {ptr->getSize(size.value_or(12))};
		}
	}

	void FontLoaderWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<FontLoaderWrapper>("fontLoader",
		    sol::no_constructor,
		    "get", &FontLoaderWrapper::get);
	}

} // namespace rawrbox
