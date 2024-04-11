
#include <rawrbox/render/scripting/wrappers/textures/base.hpp>
#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	void TextureWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::TextureBase>("TextureBase")

		    // UTILS----
		    .addFunction("hasTransparency", &rawrbox::TextureBase::hasTransparency)
		    .addFunction("getSize", &rawrbox::TextureBase::getSize)

		    .addFunction("getTextureID", &rawrbox::TextureBase::getTextureID)
		    .addFunction("getDepthTextureID", &rawrbox::TextureBase::getDepthTextureID)

		    .addFunction("setSlice", &rawrbox::TextureBase::setSlice)
		    .addFunction("getSlice", &rawrbox::TextureBase::getSlice)

		    .addFunction("getChannels", &rawrbox::TextureBase::getChannels)

		    .addFunction("setName", &rawrbox::TextureBase::setName)
		    .addFunction("getName", &rawrbox::TextureBase::getName)

		    .addFunction("isRegistered", &rawrbox::TextureBase::isRegistered)
		    .addFunction("isValid", &rawrbox::TextureBase::isValid)
		    // -----

		    .endClass();
	}
} // namespace rawrbox
