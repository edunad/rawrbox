#pragma once

#include <rawrbox/render/models/animation.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class AnimationWrapper {
	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
