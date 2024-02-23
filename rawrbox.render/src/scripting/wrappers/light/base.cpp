
#include <rawrbox/render/lights/base.hpp>
#include <rawrbox/render/scripting/wrappers/light/base.hpp>
namespace rawrbox {
	void LightBaseWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::LightBase>("Light")

		    .addFunction("setColor", &LightBase::setColor)
		    .addFunction("getColor", &LightBase::getColor)

		    .addFunction("getData", &LightBase::getData)

		    .addFunction("getRadius", &LightBase::getRadius)
		    .addFunction("setRadius", &LightBase::setRadius)

		    .addFunction("getIntensity", &LightBase::getIntensity)
		    .addFunction("setIntensity", &LightBase::setIntensity)

		    .addFunction("id", &LightBase::id)

		    .addFunction("isActive", &LightBase::isActive)
		    .addFunction("setActive", &LightBase::setActive)

		    .addFunction("getPos", &LightBase::getPos)
		    .addFunction("setPos", &LightBase::setPos)

		    .addFunction("getOffsetPos", &LightBase::getOffsetPos)
		    .addFunction("setOffsetPos", &LightBase::setOffsetPos)

		    .addFunction("getWorldPos", &LightBase::getWorldPos)
		    .addFunction("getType", &LightBase::getType)

		    .addFunction("getDirection", &LightBase::getDirection)
		    .addFunction("setDirection", &LightBase::setDirection)
		    .endClass();
	}
} // namespace rawrbox
