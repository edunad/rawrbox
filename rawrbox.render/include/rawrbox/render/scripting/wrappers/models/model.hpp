#pragma once

#include <rawrbox/render/models/model.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class ModelWrapper {
		template <typename T = rawrbox::MaterialUnlit>
			requires(std::derived_from<T, rawrbox::MaterialBase>)
		static void registerTemplate(lua_State* L, const std::string& name) {
			using ModelC = rawrbox::Model<T>;
			using ModelBaseC = rawrbox::ModelBase<T>;

			luabridge::getGlobalNamespace(L)
			    .deriveClass<ModelC, ModelBaseC>(name.c_str())
			    // OPTIMIZATION ----
			    .addFunction("flattenMeshes", &ModelC::flattenMeshes)
			    .addFunction("setMergeable", &ModelC::setMergeable)
			    .addFunction("optimize", &ModelC::optimize)
			    // ----

			    // ANIMATION ----
			    .addFunction("playAnimation", &ModelC::playAnimation)
			    .addFunction("hasAnimation", &ModelC::playAnimation)
			    .addFunction("stopAnimation", &ModelC::stopAnimation)
			    .addFunction("stopAllAnimations", &ModelC::stopAllAnimations)
			    .addFunction("isAnimationPlaying", &ModelC::isAnimationPlaying)
			    // ----

			    // UTILS ----
			    .addFunction("addMesh", &ModelC::addMesh)
			    .addFunction("totalMeshes", &ModelC::totalMeshes)
			    .addFunction("empty", &ModelC::empty)
			    .addFunction("removeMeshByName", &ModelC::removeMeshByName)
			    .addFunction("removeMesh", &ModelC::removeMesh)
			    .addFunction("getMeshByName", &ModelC::getMeshByName)
			    .addFunction("getMesh", &ModelC::getMesh)
			    .addFunction("hasMesh", &ModelC::hasMesh)
			    .addFunction("setCulling", [](ModelC& self, uint32_t cull, int id = -1) {
				    self.setCulling(static_cast<Diligent::CULL_MODE>(cull), id);
			    })
			    .addFunction("setWireframe", &ModelC::setWireframe)
			    .addFunction("setTexture", &ModelC::setTexture)
			    // ----

			    .endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
