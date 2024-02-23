#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class DecalWrapper {

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
