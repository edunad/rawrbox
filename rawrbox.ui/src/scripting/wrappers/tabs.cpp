#include <rawrbox/ui/elements/tabs.hpp>
#include <rawrbox/ui/scripting/wrappers/tabs.hpp>

namespace rawrbox {
	void UITabsWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UITabs, rawrbox::UIContainer>("UITabs")
		    .addConstructor<void(rawrbox::UIRoot*, const std::vector<rawrbox::UITab>&)>()

		    .addFunction("setActive", &rawrbox::UITabs::setActive)
		    .addFunction("setEnabled", &rawrbox::UITabs::setEnabled)

		    .addFunction("getButtonWidth", &rawrbox::UITabs::getButtonWidth)
		    .addFunction("getButtonFontSize", &rawrbox::UITabs::getButtonFontSize)
		    .addFunction("getTabHeight", &rawrbox::UITabs::getTabHeight)

		    .endClass();
	}
} // namespace rawrbox
