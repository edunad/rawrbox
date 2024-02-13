
#include <rawrbox/render/materials/instancedLit.hpp>
#include <rawrbox/render/scripting/wrappers/models/instanced.hpp>

namespace rawrbox {
	void InstancedModelWrapper::registerLua(lua_State* L) {
		registerTemplate<rawrbox::MaterialInstanced>(L, "InstancedModel");
		registerTemplate<rawrbox::MaterialInstancedLit>(L, "InstancedModelLit");
	}
} // namespace rawrbox
