
#include <rawrbox/render/materials/instancedLit.hpp>
#include <rawrbox/render/scripting/wrappers/models/instanced.hpp>

namespace rawrbox {
	void InstancedModelWrapper::registerLua(lua_State* L) {
		InstancedModelWrapper::registerTemplate<rawrbox::MaterialInstanced>(L, "InstancedModel");
		InstancedModelWrapper::registerTemplate<rawrbox::MaterialInstancedLit>(L, "InstancedModelLit");
	}
} // namespace rawrbox
