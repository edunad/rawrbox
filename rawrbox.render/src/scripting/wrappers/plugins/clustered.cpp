#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/scripting/wrappers/plugins/clustered.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	rawrbox::ClusteredPlugin* ClusteredPluginWrapper::get() {
		if (rawrbox::RENDERER == nullptr) return nullptr;
		return rawrbox::RENDERER->getPlugin<rawrbox::ClusteredPlugin>("Clustered");
	}

	void ClusteredPluginWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::ClusteredPlugin>("ClusteredPlugin")

		    // CAMERAS ----
		    .addFunction("addCamera", &ClusteredPlugin::addCamera)
		    .addFunction("removeCamera", &ClusteredPlugin::removeCamera)
		    .addFunction("isBinned", &ClusteredPlugin::isBinned)
		    // ----------------

		    .endClass();
	}
} // namespace rawrbox
