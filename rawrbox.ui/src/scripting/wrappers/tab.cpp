#include <rawrbox/ui/elements/tabs.hpp>
#include <rawrbox/ui/scripting/wrappers/tab.hpp>

namespace rawrbox {
	void UITabWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::UITab>("UITab")
		    .addConstructor<void(rawrbox::UITab), void(const std::string&, const std::string&, rawrbox::UIGroup*)>()

		    .addProperty("id", &rawrbox::UITab::id, nullptr)
		    .addProperty("name", &rawrbox::UITab::name, nullptr)
		    .addProperty("group", &rawrbox::UITab::group, nullptr)

		    .endClass();
	}
} // namespace rawrbox
