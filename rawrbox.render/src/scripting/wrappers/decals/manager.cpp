
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/scripting/wrappers/decals/manager.hpp>

namespace rawrbox {
	void DecalsWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("decals")
		    .addFunction("get", &DECALS::get)
		    .addFunction("count", &DECALS::count)
		    .addFunction("add", &DECALS::add)

		    .addFunction("remove",
			luabridge::overload<const rawrbox::Decal&>(&DECALS::remove),
			luabridge::overload<size_t>(&DECALS::remove))

		    .endNamespace();
	}
} // namespace rawrbox
