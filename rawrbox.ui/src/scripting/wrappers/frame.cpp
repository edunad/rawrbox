#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/scripting/wrappers/frame.hpp>

namespace rawrbox {
	void UIFrameWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UIFrame, rawrbox::UIContainer>("UIFrame")
		    .addFunction("getTitleColor", &rawrbox::UIFrame::getTitleColor)
		    .addFunction("setTitleColor", &rawrbox::UIFrame::setTitleColor)

		    .addFunction("setTitle", &rawrbox::UIFrame::setTitle)
		    .addFunction("getTitle", &rawrbox::UIFrame::getTitle)

		    .addFunction("setClosable", &rawrbox::UIFrame::setClosable)
		    .addFunction("isClosable", &rawrbox::UIFrame::isClosable)

		    .addFunction("setDraggable", &rawrbox::UIFrame::setDraggable)
		    .addFunction("isDraggable", &rawrbox::UIFrame::isDraggable)
		    .addFunction("onClose", [](rawrbox::UIFrame& self, const luabridge::LuaRef& callback) {
			    if (!callback.isCallable()) throw std::runtime_error("Callback not a function");

			    self.onClose += [callback]() -> void {
				    auto result = luabridge::call(callback);
				    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
			    };
		    })
		    .endClass();
	}
} // namespace rawrbox
