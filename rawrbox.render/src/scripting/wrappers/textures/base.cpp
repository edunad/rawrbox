
#include <rawrbox/render/scripting/wrappers/textures/base.hpp>
#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	void TextureWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::TextureBase>("TextureBase")

		    // UTILS----
		    .addFunction("hasTransparency", &rawrbox::TextureBase::hasTransparency)
		    .addFunction("getSize", &rawrbox::TextureBase::getSize)
		    .addFunction("isValid", &rawrbox::TextureBase::isValid)
		    // -----

		    .endClass();
	}
} // namespace rawrbox
