#pragma once

#include <rawrbox/render/models/base.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class ModelBaseWrapper {
		template <typename T = rawrbox::MaterialUnlit>
			requires(std::derived_from<T, rawrbox::MaterialBase>)
		static void registerTemplate(lua_State* L, const std::string& name) {
			using ModelC = rawrbox::ModelBase<T>;

			luabridge::getGlobalNamespace(L)
			    .beginClass<ModelC>(name.c_str())

			    // Blend shapes ---
			    .addFunction("createBlendShape", &ModelC::createBlendShape)
			    .addFunction("removeBlendShape", &ModelC::removeBlendShape)
			    .addFunction("setBlendShape", &ModelC::setBlendShape)
			    .addFunction("setBlendShapeByKey", &ModelC::setBlendShapeByKey)
			    // ----

			    .addFunction("updateBuffers", &ModelC::updateBuffers)

			    .addFunction("getColor", &ModelC::getColor)
			    .addFunction("setColor", &ModelC::setColor)

			    .addFunction("getPos", &ModelC::getPos)
			    .addFunction("setPos", &ModelC::setPos)

			    .addFunction("getScale", &ModelC::getScale)
			    .addFunction("setScale", &ModelC::setScale)

			    .addFunction("getAngle", &ModelC::getAngle)
			    .addFunction("setAngle", &ModelC::setAngle)
			    .addFunction("setEulerAngle", &ModelC::setEulerAngle)

			    .addFunction("getMatrix", &ModelC::getMatrix)

			    .addFunction("isDynamic", &ModelC::isDynamic)
			    .addFunction("isUploaded", &ModelC::isUploaded)

			    .endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
