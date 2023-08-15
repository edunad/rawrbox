#pragma once

#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/scripting/wrappers/resources/texture_loader_wrapper.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	TextureLoaderWrapper::TextureLoaderWrapper(rawrbox::Mod* mod) : _mod(mod) {}

	rawrbox::TextureWrapper TextureLoaderWrapper::get(const std::string& path, sol::optional<uint32_t> loadFlags) {
		if (this->_mod == nullptr) std::runtime_error("[RawrBox-TextureLoader] MOD not set!");
		auto fixedPath = rawrbox::LuaUtils::getContent(path, this->_mod->getFolder().generic_string());

		if (!rawrbox::RESOURCES::isLoaded(fixedPath)) {
			fmt::print("[Resources] Loading '{}' RUNTIME! You should load content on the 'MOD:onLoad' stage instead!", fixedPath);

			auto ptr = rawrbox::RESOURCES::loadFile<rawrbox::ResourceTexture>(fixedPath, loadFlags.value_or(0));
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[Resources] '{}' not found!", fixedPath));

			return {ptr->get()};
		} else {
			auto ptr = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(fixedPath);
			if (ptr == nullptr) throw std::runtime_error(fmt::format("[Resources] '{}' not found!", fixedPath));

			return {ptr->get()};
		}
	}

	void TextureLoaderWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<TextureLoaderWrapper>("textureLoader",
		    sol::no_constructor,
		    "get", &TextureLoaderWrapper::get);
	}

} // namespace rawrbox
