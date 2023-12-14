
#include <rawrbox/scripting/wrappers/mod_wrapper.hpp>
#include <rawrbox/scripting/wrappers/scripting_wrapper.hpp>

namespace rawrbox {
	sol::object ScriptingWrapper::getMod(const std::string& id, sol::this_state lua) {
		auto& mods = rawrbox::SCRIPTING::getMods();

		auto fnd = mods.find(id);
		if (fnd == mods.end()) return sol::nil;

		return sol::make_object(lua, rawrbox::ModWrapper(fnd->second.get()));
	}

	void ScriptingWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::ScriptingWrapper>("scripting",
		    sol::no_constructor,
		    "getMod", &ScriptingWrapper::getMod);
	}
} // namespace rawrbox
