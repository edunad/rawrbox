
#include <rawrbox/render/models/base.hpp>
#include <rawrbox/render/scripting/wrappers/models/base.hpp>

namespace rawrbox {
	void ModelBaseWrapper::registerLua(lua_State* L) {
		using ModelC = rawrbox::ModelBase<>;

		luabridge::getGlobalNamespace(L)
		    .beginClass<ModelC>("ModelBase")

		    // Blend shapes ---
		    .addFunction("createBlendShape", &ModelC::createBlendShape)
		    .addFunction("removeBlendShape", &ModelC::removeBlendShape)
		    .addFunction("setBlendShape", &ModelC::setBlendShape)
		    .addFunction("setBlendShapeByKey", &ModelC::setBlendShapeByKey)
		    // ----

		    .addFunction("updateBuffers", &ModelC::updateBuffers)

		    .addFunction("getPos", &ModelC::getPos)
		    .addFunction("setPos", &ModelC::setPos)

		    .addFunction("getScale", &ModelC::getScale)
		    .addFunction("setScale", &ModelC::setScale)

		    .addFunction("getAngle", &ModelC::getAngle)
		    .addFunction("setAngle", &ModelC::setAngle)
		    .addFunction("setEulerAngle", &ModelC::setEulerAngle)

		    .addFunction("getMatrix", &ModelC::getMatrix)

		    .addFunction("isDynamic", &ModelC::isDynamic)
		    .addFunction("isUploaded", &ModelC::isUploaded)

		    .endClass();
	}
} // namespace rawrbox
