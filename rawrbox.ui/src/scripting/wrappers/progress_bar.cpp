#include <rawrbox/ui/elements/progress_bar.hpp>
#include <rawrbox/ui/scripting/wrappers/progress_bar.hpp>

namespace rawrbox {
	void UIProgressBarWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UIProgressBar, rawrbox::UIContainer>("UIProgressBar")
		    .addFunction("showPercent", &rawrbox::UIProgressBar::showPercent)
		    .addFunction("isPercentVisible", &rawrbox::UIProgressBar::isPercentVisible)

		    .addFunction("setBarColor", &rawrbox::UIProgressBar::setBarColor)
		    .addFunction("getBarColor", &rawrbox::UIProgressBar::getBarColor)

		    .addFunction("setValue", &rawrbox::UIProgressBar::setValue)
		    .addFunction("getValue", &rawrbox::UIProgressBar::getValue)
		    .endClass();
	}
} // namespace rawrbox
