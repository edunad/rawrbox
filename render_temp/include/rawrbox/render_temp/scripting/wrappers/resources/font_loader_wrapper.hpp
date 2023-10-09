#pragma once

#include <rawrbox/render_temp/scripting/wrappers/text/font_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class FontLoaderWrapper {

	public:
		FontLoaderWrapper() = default;
		FontLoaderWrapper(const FontLoaderWrapper&) = default;
		FontLoaderWrapper(FontLoaderWrapper&&) = default;
		FontLoaderWrapper& operator=(const FontLoaderWrapper&) = default;
		FontLoaderWrapper& operator=(FontLoaderWrapper&&) = default;
		virtual ~FontLoaderWrapper() = default;

		virtual rawrbox::FontWrapper get(const std::string& path, sol::optional<uint16_t> size, sol::this_environment modEnv);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
