#include <rawrbox/ui/elements/label.hpp>
#include <rawrbox/ui/scripting/wrappers/label.hpp>

namespace rawrbox {
	void UILabelWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UILabel, rawrbox::UIContainer>("UILabel")
		    .addFunction("setColor", &rawrbox::UILabel::setColor)
		    .addFunction("getColor", &rawrbox::UILabel::getColor)

		    .addFunction("setShadowPos", &rawrbox::UILabel::setShadowPos)
		    .addFunction("getShadowPos", &rawrbox::UILabel::getShadowPos)

		    .addFunction("setShadowColor", &rawrbox::UILabel::setShadowColor)
		    .addFunction("getShadowColor", &rawrbox::UILabel::getShadowColor)

		    .addFunction("setText", &rawrbox::UILabel::setText)
		    .addFunction("getText", &rawrbox::UILabel::getText)

		    .addFunction("setFontPTR", [](rawrbox::UILabel& self, rawrbox::Font* fnt) {
			    self.setFont(fnt);
		    })
		    .addFunction("setFontPath", [](rawrbox::UILabel& self, const std::string& path, lua_State* L) {
			    auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
			    auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

			    self.setFont(fixedPath);
		    })

		    .addFunction("getFont", &rawrbox::UILabel::getFont)
		    .addFunction("sizeToContents", &rawrbox::UILabel::sizeToContents)
		    .endClass();
	}
} // namespace rawrbox
