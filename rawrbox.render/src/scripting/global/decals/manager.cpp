
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/scripting/global/decals/manager.hpp>

namespace rawrbox {
	void DecalsGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("decals", {})

		    .addFunction("get", &DECALS::get)
		    .addFunction("count", &DECALS::count)
		    .addFunction("add", &DECALS::add)
		    .addFunction("clear", &DECALS::clear)
		    .addFunction("remove", &DECALS::remove)

		    .endNamespace();
	}
} // namespace rawrbox
