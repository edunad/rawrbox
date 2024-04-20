

#include <rawrbox/render/scripting/wrappers/models/mesh.hpp>

namespace rawrbox {
	void MeshWrapper::registerLua(lua_State* L) {
		registerTemplate<rawrbox::VertexData>(L, "MeshBase");
		registerTemplate<rawrbox::VertexUVData>(L, "Mesh");
		registerTemplate<rawrbox::VertexNormData>(L, "MeshNorm");
		registerTemplate<rawrbox::VertexBoneData>(L, "MeshBone");
		registerTemplate<rawrbox::VertexNormBoneData>(L, "MeshNormBone");
	}
} // namespace rawrbox
