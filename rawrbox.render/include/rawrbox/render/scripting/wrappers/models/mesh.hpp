#pragma once
#include <rawrbox/render/models/mesh.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class MeshWrapper {
		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		static void registerTemplate(lua_State* L, const std::string& name) {
			using MeshC = rawrbox::Mesh<T>;

			luabridge::getGlobalNamespace(L)
			    .beginClass<MeshC>(name.c_str())

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

			    .addFunction("setLineMode", &MeshC::setLineMode)
			    .addFunction("getLineMode", &MeshC::getLineMode)

			    .addFunction("setLineMode", &MeshC::setTransparent)
			    .addFunction("isTransparent", &MeshC::isTransparent)

			    .addFunction("getSlice", &MeshC::getSlice)
			    .addFunction("setSlice", &MeshC::setSlice)

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
			    .addFunction("getWireframe", &MeshC::getWireframe)

			    .addFunction("setCulling", [](MeshC& self, uint32_t cull) {
				    self.setCulling(static_cast<Diligent::CULL_MODE>(cull));
			    })

			    .addFunction("getID", &MeshC::getID)
			    .addFunction("setID", &MeshC::setID)

			    .addFunction("getColor", &MeshC::getColor)
			    .addFunction("setColor", &MeshC::setColor)

			    .addFunction("clear", &MeshC::clear)
			    .addFunction("merge", &MeshC::merge)

			    .addFunction("setMergeable", &MeshC::setMergeable)
			    .addFunction("canMerge", &MeshC::canMerge)

			    .endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
