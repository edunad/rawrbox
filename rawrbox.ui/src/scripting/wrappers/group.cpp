#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/scripting/wrappers/group.hpp>

namespace rawrbox {
	void UIGroupWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UIGroup, rawrbox::UIContainer>("UIGroup")
		    .addFunction("setBorder", &rawrbox::UIGroup::setBorder)
		    .addFunction("getBorder", &rawrbox::UIGroup::getBorder)

		    .addFunction("sizeToContents", &rawrbox::UIGroup::sizeToContents)
		    .endClass();
	}
} // namespace rawrbox
