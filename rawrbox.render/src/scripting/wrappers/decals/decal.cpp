
#include <rawrbox/render/decals/decal.hpp>
#include <rawrbox/render/scripting/wrappers/decals/decal.hpp>

namespace rawrbox {
	void DecalWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::Decal>("Decal")
		    .addConstructor<void(), void(rawrbox::Decal&), void(const rawrbox::Matrix4x4&, const rawrbox::TextureBase&, const rawrbox::Colorf&, uint32_t)>()
		    .addFunction("setTexture", &Decal::setTexture)
		    .addFunction("setScale", &Decal::setScale)
		    .addFunction("setPos", &Decal::setPos)
		    .addFunction("setColor", &Decal::setColor)
		    .endClass();
	}
} // namespace rawrbox
