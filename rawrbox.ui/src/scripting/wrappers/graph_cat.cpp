#include <rawrbox/ui/elements/graph.hpp>
#include <rawrbox/ui/scripting/wrappers/graph_cat.hpp>

namespace rawrbox {
	void UIGraphCatWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::UIGraphCategory>("UIGraphCategory")

		    .addFunction("getName", &rawrbox::UIGraphCategory::getName)
		    .addFunction("setName", &rawrbox::UIGraphCategory::setName)

		    .addFunction("getColor", &rawrbox::UIGraphCategory::getColor)
		    .addFunction("setColor", &rawrbox::UIGraphCategory::setColor)

		    .addFunction("addEntry", &rawrbox::UIGraphCategory::addEntry)

		    .addFunction("startTimer", &rawrbox::UIGraphCategory::startTimer)
		    .addFunction("stopTimer", &rawrbox::UIGraphCategory::stopTimer)
		    .endClass();
	}
} // namespace rawrbox
