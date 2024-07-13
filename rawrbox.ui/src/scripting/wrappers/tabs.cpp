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

		    .addFunction("onTabChange", [](rawrbox::UITabs& self, const luabridge::LuaRef& callback) {
			    if (!callback.isCallable()) throw std::runtime_error("Callback not a function");

			    self.onTabChange += [callback](const std::string& tabId) -> void {
				    auto result = luabridge::call(callback, tabId);
				    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
			    };
		    })

		    .endClass();
	}
} // namespace rawrbox
