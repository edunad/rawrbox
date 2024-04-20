

#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/materials/skinnedLit.hpp>
#include <rawrbox/render/scripting/wrappers/models/model.hpp>

namespace rawrbox {
	void ModelWrapper::registerLua(lua_State* L) {
		registerTemplate<rawrbox::MaterialUnlit>(L, "Model");
		registerTemplate<rawrbox::MaterialLit>(L, "ModelLit");
		registerTemplate<rawrbox::MaterialSkinned>(L, "ModelSkinned");
		registerTemplate<rawrbox::MaterialSkinnedLit>(L, "ModelSkinnedLit");
	}
} // namespace rawrbox
