#pragma once
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class UIRoot;
	class UIWrapper {
	public:
		static void registerLua(lua_State* L, rawrbox::UIRoot* root);
	};
} // namespace rawrbox
