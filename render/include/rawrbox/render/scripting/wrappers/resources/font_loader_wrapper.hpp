#pragma once

#include <rawrbox/render/scripting/wrappers/text/font_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class FontLoaderWrapper {
		rawrbox::Mod* _mod = nullptr;

	public:
		FontLoaderWrapper(rawrbox::Mod* mod_);
		FontLoaderWrapper(const FontLoaderWrapper&) = default;
		FontLoaderWrapper(FontLoaderWrapper&&) = default;
		FontLoaderWrapper& operator=(const FontLoaderWrapper&) = default;
		FontLoaderWrapper& operator=(FontLoaderWrapper&&) = default;
		virtual ~FontLoaderWrapper() = default;

		virtual rawrbox::FontWrapper get(const std::string& path, sol::optional<uint32_t> size);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
