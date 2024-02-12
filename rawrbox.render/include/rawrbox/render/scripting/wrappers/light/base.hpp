#pragma once

#include <rawrbox/render/lights/base.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class LightBaseWrapper {

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
