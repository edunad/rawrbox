
#include <rawrbox/render/scripting/wrappers/text/font.hpp>
#include <rawrbox/render/text/font.hpp>
namespace rawrbox {
	void FontWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::Font>("Font")

		    .addFunction("getSize", &rawrbox::Font::getSize)
		    .addFunction("getLineHeight", &rawrbox::Font::getLineHeight)
		    .addFunction("getStringSize", &rawrbox::Font::getStringSize)
		    .addFunction("hasGlyph", &rawrbox::Font::hasGlyph)

		    .endClass();
	}
} // namespace rawrbox
