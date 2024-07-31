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

		    .addFunction("setFont", [](rawrbox::UILabel& self, rawrbox::Font* fnt) {
			    self.setFont(fnt);
		    })
		    .addFunction("setFontPath", [](rawrbox::UILabel& self, const std::string& path, lua_State* L) {
			    auto modFolder = rawrbox::LuaUtils::getLuaENVVar<std::string>(L, "__mod_folder");
			    auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);
			    if (!fixedPath.first.empty()) throw std::runtime_error("External mod resources loading not supported");

			    self.setFont(fixedPath.second);
		    })

		    .addFunction("getFont", &rawrbox::UILabel::getFont)
		    .addFunction("sizeToContents", &rawrbox::UILabel::sizeToContents)
		    .endClass();
	}
} // namespace rawrbox
