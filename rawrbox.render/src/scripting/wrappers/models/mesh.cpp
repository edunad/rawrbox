
#include <rawrbox/render/models/mesh.hpp>
#include <rawrbox/render/scripting/wrappers/models/mesh.hpp>

namespace rawrbox {
	void MeshWrapper::registerLua(lua_State* L) {
		using MeshC = rawrbox::Mesh<>;

		luabridge::getGlobalNamespace(L)
		    .beginClass<MeshC>("Mesh")

		    .addProperty("baseVertex", &MeshC::baseVertex)
		    .addProperty("baseIndex", &MeshC::baseIndex)
		    .addProperty("totalVertex", &MeshC::totalVertex)
		    .addProperty("totalIndex", &MeshC::totalIndex)

		    // TODO
		    //.addProperty("vertices", &MeshC::vertices)
		    //.addProperty("indices", &MeshC::indices)
		    //.addProperty("textures", &MeshC::textures)

		    .addFunction("getName", &MeshC::getName)
		    .addFunction("setName", &MeshC::setName)

		    .addFunction("getBBOX", &MeshC::getBBOX)

		    .addFunction("empty", &MeshC::empty)
		    .addFunction("getMatrix", &MeshC::getMatrix)

		    .addFunction("getPos", &MeshC::getPos)
		    .addFunction("setPos", &MeshC::setPos)

		    .addFunction("getAngle", &MeshC::getAngle)
		    .addFunction("setAngle", &MeshC::setAngle)
		    .addFunction("setEulerAngle", &MeshC::setEulerAngle)

		    .addFunction("getScale", &MeshC::getScale)
		    .addFunction("setScale", &MeshC::setScale)

		    .addFunction("setTransparentBlending", &MeshC::setTransparentBlending)

		    .addFunction("getAtlasID", &MeshC::getAtlasID)
		    .addFunction("setAtlasID", &MeshC::setAtlasID)

		    .addFunction("getTexture", &MeshC::getTexture)
		    .addFunction("setTexture", &MeshC::setTexture)

		    .addFunction("getNormalTexture", &MeshC::getNormalTexture)
		    .addFunction("setNormalTexture", &MeshC::setNormalTexture)

		    .addFunction("getDisplacementTexture", &MeshC::getDisplacementTexture)
		    .addFunction("setDisplacementTexture", &MeshC::setDisplacementTexture)

		    .addFunction("getEmissionTexture", &MeshC::getEmissionTexture)
		    .addFunction("setEmissionTexture", &MeshC::setEmissionTexture)

		    .addFunction("getRoughtMetalTexture", &MeshC::getRoughtMetalTexture)
		    .addFunction("setRoughtMetalTexture", &MeshC::setRoughtMetalTexture)

		    .addFunction("setSpecularFactor", &MeshC::setSpecularFactor)
		    .addFunction("setBillboard", &MeshC::setBillboard)
		    .addFunction("setVertexSnap", &MeshC::setVertexSnap)
		    .addFunction("setWireframe", &MeshC::setWireframe)
		    .addFunction("setCulling", [](MeshC& self, uint32_t cull) {
			    self.setCulling(static_cast<Diligent::CULL_MODE>(cull));
		    })

		    .addFunction("getId", &MeshC::getId)
		    .addFunction("setId", &MeshC::setId)

		    .addFunction("getColor", &MeshC::getColor)
		    .addFunction("setColor", &MeshC::setColor)

		    .addFunction("clear", &MeshC::clear)
		    .addFunction("merge", &MeshC::merge)

		    .addFunction("rotateVertices", &MeshC::rotateVertices)
		    .addFunction("setOptimizable", &MeshC::setOptimizable)
		    .addFunction("canOptimize", &MeshC::canOptimize)

		    .endClass();
	}
} // namespace rawrbox
