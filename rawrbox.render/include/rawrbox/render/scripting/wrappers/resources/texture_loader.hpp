#pragma once

#include <rawrbox/render/text/font.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class TextureLoaderWrapper {

	public:
		static rawrbox::TextureBase* get(const std::string& path, std::optional<uint32_t> loadFlags, lua_State* L);
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
