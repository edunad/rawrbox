#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/ui/scripting/wrappers/input.hpp>

namespace rawrbox {
	void UIInputWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UIInput, rawrbox::UIContainer>("UIInput")

		    .addFunction("setHints", &rawrbox::UIInput::setHints)

		    .addFunction("setText", &rawrbox::UIInput::setText)
		    .addFunction("getText", &rawrbox::UIInput::getText)

		    .addFunction("setPlaceholder", &rawrbox::UIInput::setPlaceholder)
		    .addFunction("getPlaceholder", &rawrbox::UIInput::getPlaceholder)

		    .addFunction("setLimit", &rawrbox::UIInput::setLimit)
		    .addFunction("getLimit", &rawrbox::UIInput::getLimit)

		    .addFunction("setFill", &rawrbox::UIInput::setFill)

		    .addFunction("setNumericOnly", &rawrbox::UIInput::setNumericOnly)
		    .addFunction("getNumericOnly", &rawrbox::UIInput::getNumericOnly)

		    .addFunction("setReadOnly", &rawrbox::UIInput::setReadOnly)
		    .addFunction("getReadOnly", &rawrbox::UIInput::getReadOnly)

		    .addFunction("setPadding", &rawrbox::UIInput::setPadding)
		    .addFunction("getPadding", &rawrbox::UIInput::getPadding)

		    .addFunction("setColor", &rawrbox::UIInput::setColor)
		    .addFunction("getColor", &rawrbox::UIInput::getColor)

		    .addFunction("setBorderSize", &rawrbox::UIInput::setBorderSize)
		    .addFunction("setBorderColor", &rawrbox::UIInput::setBorderColor)
		    .addFunction("getBorderColor", &rawrbox::UIInput::getBorderColor)

		    .addFunction("setBackgroundColor", &rawrbox::UIInput::setBackgroundColor)
		    .addFunction("getBackgroundColor", &rawrbox::UIInput::getBackgroundColor)

		    .addFunction("setFontPTR", [](rawrbox::UIInput& self, rawrbox::Font* fnt) {
			    self.setFont(fnt);
		    })
		    .addFunction("setFontPath", [](rawrbox::UIInput& self, const std::string& path, std::optional<uint16_t> size, lua_State* L) {
			    auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
			    auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

			    self.setFont(fixedPath, size.value_or(11));
		    })

		    .addFunction("getFont", &rawrbox::UIInput::getFont)

		    .addFunction("empty", &rawrbox::UIInput::empty)
		    .addFunction("size", &rawrbox::UIInput::size)
		    .addFunction("clear", &rawrbox::UIInput::clear)

		    .addFunction("onKey", [](rawrbox::UIInput& self, const luabridge::LuaRef& callback) {
			    if (!callback.isCallable()) throw std::runtime_error("Callback not a function");

			    self.onKey += [callback](uint32_t key) -> void {
				    auto result = luabridge::call(callback, key);
				    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
			    };
		    })
		    .addFunction("onTextUpdate", [](rawrbox::UIInput& self, const luabridge::LuaRef& callback) {
			    if (!callback.isCallable()) throw std::runtime_error("Callback not a function");

			    self.onTextUpdate += [callback]() -> void {
				    auto result = luabridge::call(callback);
				    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
			    };
		    })
		    .addFunction("onEnter", [](rawrbox::UIInput& self, const luabridge::LuaRef& callback) {
			    if (!callback.isCallable()) throw std::runtime_error("Callback not a function");

			    self.onEnter += [callback]() -> void {
				    auto result = luabridge::call(callback);
				    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
			    };
		    })
		    .endClass();
	}
} // namespace rawrbox
