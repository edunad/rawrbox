#include <rawrbox/ui/elements/loading.hpp>
#include <rawrbox/ui/scripting/wrappers/loading.hpp>

namespace rawrbox {
	void UILoadingWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UILoading, rawrbox::UIContainer>("UILoading")
		    .endClass();
	}
} // namespace rawrbox
