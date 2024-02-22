#pragma once

#include <rawrbox/render/models/instanced.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class InstancedModelWrapper {
		template <typename T = rawrbox::MaterialInstanced>
			requires(std::derived_from<T, rawrbox::MaterialBase>)
		static void registerTemplate(lua_State* L, const std::string& name) {
			using ModelC = rawrbox::InstancedModel<T>;
			using ModelBaseC = rawrbox::ModelBase<T>;

			luabridge::getGlobalNamespace(L)
			    .deriveClass<ModelC, ModelBaseC>(name.c_str())
			    .addFunction("setAutoUpload", &ModelC::setAutoUpload)

			    .addFunction("setTemplate", &ModelC::setTemplate)
			    .addFunction("getTemplate", &ModelC::getTemplate)

			    .addFunction("addInstance", &ModelC::addInstance)
			    .addFunction("removeInstance", &ModelC::removeInstance)
			    .addFunction("getInstance", &ModelC::getInstance)

			    .addFunction("count", &ModelC::count)
			    .endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
