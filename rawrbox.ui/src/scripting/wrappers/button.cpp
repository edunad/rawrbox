
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/scripting/wrappers/button.hpp>

namespace rawrbox {
	void UIButtonWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UIButton, rawrbox::UIContainer>("UIButton")

		    .addFunction("setTextureSize", &rawrbox::UIButton::setTextureSize)

		    .addFunction("setTextureColor", &rawrbox::UIButton::setTextureColor)
		    .addFunction("getTextureColor", &rawrbox::UIButton::getTextureColor)

		    .addFunction("setTextColor", &rawrbox::UIButton::setTextColor)
		    .addFunction("getTextColor", &rawrbox::UIButton::getTextColor)

		    .addFunction("setBackgroundColor", &rawrbox::UIButton::setBackgroundColor)
		    .addFunction("getBackgroundColor", &rawrbox::UIButton::getBackgroundColor)

		    .addFunction("setText", [](rawrbox::UIButton& self, const std::string& str, std::optional<uint16_t> size) {
			    self.setText(str, size.value_or(16U));
		    })
		    .addFunction("getText", &rawrbox::UIButton::getText)

		    .addFunction("setTooltip", &rawrbox::UIButton::setTooltip)
		    .addFunction("getTooltip", &rawrbox::UIButton::getTooltip)

		    .addFunction("setTexturePTR", [](rawrbox::UIButton& self, rawrbox::TextureBase* tex) {
			    self.setTexture(tex);
		    })
		    .addFunction("setTexturePath", [](rawrbox::UIButton& self, const std::string& path, lua_State* L) {
			    auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");
			    auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

			    self.setTexture(fixedPath);
		    })

		    .addFunction("setEnabled", &rawrbox::UIButton::setEnabled)
		    .addFunction("isEnabled", &rawrbox::UIButton::isEnabled)

		    .addFunction("setBorder", &rawrbox::UIButton::setBorder)
		    .addFunction("borderEnabled", &rawrbox::UIButton::borderEnabled)
		    .addFunction("onClick", [](rawrbox::UIButton& self, const luabridge::LuaRef& callback) {
			    if (!callback.isCallable()) throw std::runtime_error("Callback not a function");

			    self.onClick += [callback]() -> void {
				    auto result = luabridge::call(callback);
				    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
			    };
		    })
		    .endClass();
	}
} // namespace rawrbox
