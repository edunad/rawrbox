
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/scripting/wrapper/mesh_wrapper.hpp>

namespace rawrbox {
	void MeshWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::Mesh>("Mesh",
		    sol::constructors<rawrbox::Mesh(), rawrbox::Mesh(rawrbox::Mesh)>(),

		    "getName", &Mesh::getName,
		    "setName", &Mesh::setName,

		    // "getVertices", &Mesh::getVertices,
		    "getIndices", &Mesh::getIndices,
		    "getBBOX", &Mesh::getBBOX,

		    "empty", &Mesh::empty,

		    "getMatrix", &Mesh::getMatrix,

		    "getPos", &Mesh::getPos,
		    "setPos", &Mesh::setPos,

		    "getAngle", &Mesh::getAngle,
		    "setAngle", &Mesh::setAngle,
		    "setEulerAngle", &Mesh::setEulerAngle,

		    "getScale", &Mesh::getScale,
		    "setScale", &Mesh::setScale,

		    /* "getTexture", &Mesh::getTexture,
		     "setTexture", &Mesh::setTexture,*/

		    "getAtlasID", &Mesh::getAtlasID,
		    "setAtlasID", &Mesh::setAtlasID,

		    /*"getNormalTexture", &Mesh::getNormalTexture,
		    "setNormalTexture", &Mesh::setNormalTexture,
		    "getDisplacementTexture", &Mesh::getDisplacementTexture,
		    "setDisplacementTexture", &Mesh::setDisplacementTexture,
		    "getEmissionTexture", &Mesh::getEmissionTexture,
		    "setEmissionTexture", &Mesh::setEmissionTexture,
		    "getSpecularTexture", &Mesh::getSpecularTexture,
		    "setSpecularTexture", &Mesh::setSpecularTexture,*/

		    "setVertexSnap", &Mesh::setVertexSnap,
		    "setWireframe", &Mesh::setWireframe,
		    "setCulling", &Mesh::setCulling,
		    "setDepthTest", &Mesh::setDepthTest,
		    "setBlend", &Mesh::setBlend,
		    "setRecieveDecals", &Mesh::setRecieveDecals,
		    "setColor", &Mesh::setColor,

		    "addData", &Mesh::addData,
		    "getData", &Mesh::getData,
		    "hasData", &Mesh::hasData,

		    //"getSkeleton", &Mesh::getSkeleton,

		    "clear", &Mesh::clear,

		    "merge", &Mesh::merge,

		    "setOptimizable", &Mesh::setOptimizable,
		    "canOptimize", &Mesh::canOptimize);
	}
} // namespace rawrbox
