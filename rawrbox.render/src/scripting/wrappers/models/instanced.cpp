
#include <rawrbox/render/models/instanced.hpp>
#include <rawrbox/render/scripting/wrappers/models/instanced.hpp>

namespace rawrbox {
	void InstancedModelWrapper::registerLua(lua_State* L) {
		using ModelC = rawrbox::InstancedModel<>;
		using ModelBaseC = rawrbox::ModelBase<>;

		luabridge::getGlobalNamespace(L)
		    .deriveClass<ModelC, ModelBaseC>("InstancedModel")
		    .addFunction("setAutoUpload", &ModelC::setAutoUpload)

		    //.addFunction("setTemplate", &ModelC::setTemplate)
		    //.addFunction("getTemplate", &ModelC::getTemplate)

		    .addFunction("addInstance", &ModelC::addInstance)
		    .addFunction("removeInstance", &ModelC::removeInstance)
		    .addFunction("getInstance", &ModelC::getInstance)

		    .addFunction("count", &ModelC::count)
		    .endClass();
	}
} // namespace rawrbox
