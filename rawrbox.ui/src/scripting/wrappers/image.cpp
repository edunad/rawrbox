#include <rawrbox/ui/elements/image.hpp>
#include <rawrbox/ui/scripting/wrappers/image.hpp>

namespace rawrbox {
	void UIImageWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UIImage, rawrbox::UIContainer>("UIImage")
		    .addFunction("setColor", &rawrbox::UIImage::setColor)
		    .addFunction("getColor", &rawrbox::UIImage::getColor)

		    .addFunction("getTexture", &rawrbox::UIImage::getTexture)
		    .addFunction("setTexturePTR", [](rawrbox::UIImage& self, rawrbox::TextureBase* tex) {
			    self.setTexture(tex);
		    })
		    .addFunction("setTexturePath", [](rawrbox::UIImage& self, const std::string& path, lua_State* L) {
			    auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
			    auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

			    self.setTexture(fixedPath);
		    })
		    .addFunction("sizeToContents", &rawrbox::UIImage::sizeToContents)
		    .endClass();
	}
} // namespace rawrbox
