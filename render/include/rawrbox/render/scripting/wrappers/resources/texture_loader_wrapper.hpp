#pragma once

#include <rawrbox/render/scripting/wrappers/textures/base_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class TextureLoaderWrapper {
		rawrbox::Mod* _mod = nullptr;

	public:
		TextureLoaderWrapper(rawrbox::Mod* mod_);
		TextureLoaderWrapper(const TextureLoaderWrapper&) = default;
		TextureLoaderWrapper(TextureLoaderWrapper&&) = default;
		TextureLoaderWrapper& operator=(const TextureLoaderWrapper&) = default;
		TextureLoaderWrapper& operator=(TextureLoaderWrapper&&) = default;
		virtual ~TextureLoaderWrapper() = default;

		virtual rawrbox::TextureWrapper get(const std::string& path, sol::optional<uint32_t> loadFlags);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
