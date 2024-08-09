#include <rawrbox/ui/elements/dropdown.hpp>
#include <rawrbox/ui/scripting/wrappers/dropdown.hpp>

namespace rawrbox {
	void UIDropdownWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UIDropdown, rawrbox::UIContainer>("UIDropdown")

		    .addFunction("setOptions", &rawrbox::UIDropdown::setOptions)
		    .addFunction("addOption", &rawrbox::UIDropdown::addOption)
		    .addFunction("removeOption", &rawrbox::UIDropdown::removeOption)

		    .addFunction("setActive", &rawrbox::UIDropdown::setActive)

		    .addFunction("getSelected", &rawrbox::UIDropdown::getSelected)
		    .addFunction("getSelectedValue", &rawrbox::UIDropdown::getSelectedValue)

		    .addFunction("onSelectionChange", [](rawrbox::UIDropdown& self, const luabridge::LuaRef& callback) {
			    if (!callback.isCallable()) throw std::runtime_error("Callback not a function");

			    self.onSelectionChange += [callback](size_t index, const std::string& value) -> void {
				    auto result = luabridge::call(callback, index, value);
				    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
			    };
		    })
		    .endClass();
	}
} // namespace rawrbox
