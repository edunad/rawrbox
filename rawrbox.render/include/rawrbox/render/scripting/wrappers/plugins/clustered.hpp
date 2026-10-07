#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class ClusteredPlugin;

	class ClusteredPluginWrapper {
	public:
		[[nodiscard]] static rawrbox::ClusteredPlugin* get();
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
