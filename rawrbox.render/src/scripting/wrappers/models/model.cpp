

#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/materials/skinnedLit.hpp>
#include <rawrbox/render/scripting/wrappers/models/model.hpp>

namespace rawrbox {
	void ModelWrapper::registerLua(lua_State* L) {
		ModelWrapper::registerTemplate<rawrbox::MaterialUnlit>(L, "Model");
		ModelWrapper::registerTemplate<rawrbox::MaterialLit>(L, "ModelLit");
		ModelWrapper::registerTemplate<rawrbox::MaterialSkinned>(L, "ModelSkinned");
		ModelWrapper::registerTemplate<rawrbox::MaterialSkinnedLit>(L, "ModelSkinnedLit");
	}
} // namespace rawrbox
