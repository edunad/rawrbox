
#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/materials/instancedLit.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/materials/skinnedLit.hpp>
#include <rawrbox/render/scripting/wrappers/models/base.hpp>

namespace rawrbox {
	void ModelBaseWrapper::registerLua(lua_State* L) {
		registerTemplate<>(L, "ModelBase");
		registerTemplate<rawrbox::MaterialLit>(L, "ModelBaseLit");
		registerTemplate<rawrbox::MaterialInstanced>(L, "ModelBaseInstanced");
		registerTemplate<rawrbox::MaterialInstancedLit>(L, "ModelBaseInstancedLit");
		registerTemplate<rawrbox::MaterialSkinned>(L, "ModelBaseSkinned");
		registerTemplate<rawrbox::MaterialSkinnedLit>(L, "ModelBaseSkinnedLit");
	}
} // namespace rawrbox
