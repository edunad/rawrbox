
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/scripting/wrappers/models/model.hpp>

namespace rawrbox {
	void ModelWrapper::registerLua(lua_State* L) {
		using ModelC = rawrbox::Model<>;
		using ModelBaseC = rawrbox::ModelBase<>;

		luabridge::getGlobalNamespace(L)
		    .deriveClass<ModelC, ModelBaseC>("Model")
		    // OPTIMIZATION ----
		    .addFunction("flattenMeshes", &ModelC::flattenMeshes)
		    .addFunction("setOptimizable", &ModelC::setOptimizable)
		    .addFunction("optimize", &ModelC::optimize)
		    // ----

		    // ANIMATION ----
		    .addFunction("playAnimation", &ModelC::playAnimation)
		    .addFunction("stopAnimation", &ModelC::stopAnimation)
		    // ----

		    // UTILS ----
		    //.addFunction("addMesh", &ModelC::addMesh)
		    .addFunction("totalMeshes", &ModelC::totalMeshes)
		    .addFunction("empty", &ModelC::empty)
		    .addFunction("removeMeshByName", &ModelC::removeMeshByName)
		    .addFunction("removeMesh", &ModelC::removeMesh)
		    .addFunction("getMeshByName", &ModelC::getMeshByName)
		    .addFunction("getMesh", &ModelC::getMesh)
		    .addFunction("hasMesh", &ModelC::hasMesh)
		    .addFunction("setCulling", [](ModelC& self, uint32_t cull, int id = -1) {
			    self.setCulling(static_cast<Diligent::CULL_MODE>(cull), id);
		    })
		    .addFunction("setWireframe", &ModelC::setWireframe)
		    .addFunction("setColor", &ModelC::setColor)
		    .addFunction("setTexture", &ModelC::setTexture)
		    // ----

		    .endClass();
	}
} // namespace rawrbox
