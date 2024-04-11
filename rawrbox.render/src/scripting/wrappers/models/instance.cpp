
#include <rawrbox/render/models/instance.hpp>
#include <rawrbox/render/scripting/wrappers/models/instance.hpp>

namespace rawrbox {
	void InstanceWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::Instance>("Instance")
		    .addConstructor<void(), void(rawrbox::Instance), void(const rawrbox::Matrix4x4&, const rawrbox::Colorf&, uint16_t, uint32_t)>()

		    .addFunction("getColor", &Instance::getColor)
		    .addFunction("setColor", &Instance::setColor)

		    .addFunction("getMatrix", &Instance::getMatrix)
		    .addFunction("setMatrix", &Instance::setMatrix)

		    .addFunction("getSlice", &Instance::getSlice)
		    .addFunction("setSlice", &Instance::setSlice)

		    .addFunction("getId", &Instance::getId)
		    .addFunction("setId", &Instance::setId)

		    .endClass();
	}
} // namespace rawrbox
