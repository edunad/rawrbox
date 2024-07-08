#pragma once

#include <rawrbox/render/text/font.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class FontLoaderGlobal {

	public:
		static rawrbox::Font* get(const std::string& path, std::optional<uint16_t> size, lua_State* L);
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
