
#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/materials/instancedLit.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/materials/skinnedLit.hpp>
#include <rawrbox/render/scripting/wrappers/models/base.hpp>

namespace rawrbox {
	void ModelBaseWrapper::registerLua(lua_State* L) {
		ModelBaseWrapper::registerTemplate<>(L, "ModelBase");
		ModelBaseWrapper::registerTemplate<rawrbox::MaterialLit>(L, "ModelBaseLit");
		ModelBaseWrapper::registerTemplate<rawrbox::MaterialInstanced>(L, "ModelBaseInstanced");
		ModelBaseWrapper::registerTemplate<rawrbox::MaterialInstancedLit>(L, "ModelBaseInstancedLit");
		ModelBaseWrapper::registerTemplate<rawrbox::MaterialSkinned>(L, "ModelBaseSkinned");
		ModelBaseWrapper::registerTemplate<rawrbox::MaterialSkinnedLit>(L, "ModelBaseSkinnedLit");
	}
} // namespace rawrbox
