#include <rawrbox/ui/container.hpp>
#include <rawrbox/ui/scripting/wrappers/container.hpp>

namespace rawrbox {
	void UIContainerWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::UIContainer>("UIContainer")

		    .addFunction("setPos", &rawrbox::UIContainer::setPos)
		    .addFunction("getPos", &rawrbox::UIContainer::getPos)
		    .addFunction("getDrawOffset", &rawrbox::UIContainer::getDrawOffset)

		    .addFunction("setSize", &rawrbox::UIContainer::setSize)
		    .addFunction("getSize", &rawrbox::UIContainer::getSize)
		    .addFunction("getContentSize", &rawrbox::UIContainer::getContentSize)
		    .addFunction("sizeToParent", &rawrbox::UIContainer::sizeToParent)

		    .addFunction("removeChildren", &rawrbox::UIContainer::removeChildren)
		    .addFunction("remove", &rawrbox::UIContainer::remove)

		    .addFunction("setVisible", &rawrbox::UIContainer::setVisible)
		    .addFunction("visible", &rawrbox::UIContainer::visible)

		    .addFunction("focus", &rawrbox::UIContainer::focus)
		    .addFunction("setFocused", &rawrbox::UIContainer::setFocused)
		    .addFunction("focused", &rawrbox::UIContainer::focused)

		    .addFunction("setHovering", &rawrbox::UIContainer::setHovering)
		    .addFunction("hovering", &rawrbox::UIContainer::hovering)

		    .addFunction("getPosAbsolute", &rawrbox::UIContainer::getPosAbsolute)

		    .addFunction("alwaysOnTop", &rawrbox::UIContainer::alwaysOnTop)
		    .addFunction("setAlwaysTop", &rawrbox::UIContainer::setAlwaysTop)
		    .addFunction("bringToFront", &rawrbox::UIContainer::bringToFront)

		    .addFunction("hasChildren", &rawrbox::UIContainer::hasChildren)
		    .endClass();
	}
} // namespace rawrbox
