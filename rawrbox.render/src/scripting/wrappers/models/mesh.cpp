

#include <rawrbox/render/scripting/wrappers/models/mesh.hpp>

namespace rawrbox {
	void MeshWrapper::registerLua(lua_State* L) {
		MeshWrapper::registerTemplate<rawrbox::VertexData>(L, "MeshBase");
		MeshWrapper::registerTemplate<rawrbox::VertexUVData>(L, "Mesh");
		MeshWrapper::registerTemplate<rawrbox::VertexNormData>(L, "MeshNorm");
		MeshWrapper::registerTemplate<rawrbox::VertexBoneData>(L, "MeshBone");
		MeshWrapper::registerTemplate<rawrbox::VertexNormBoneData>(L, "MeshNormBone");
	}
} // namespace rawrbox
