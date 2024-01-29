
#include <rawrbox/scripting/wrappers/i18n.hpp>
#include <rawrbox/utils/i18n.hpp>

namespace rawrbox {
	const std::string& I18NWrapper::getLanguage() {
		return rawrbox::I18N::getLanguage();
	}

	void I18NWrapper::setLanguage(const std::string& language) {
		return rawrbox::I18N::setLanguage(language);
	}

	std::string I18NWrapper::get(const luabridge::LuaRef& ref) {
		auto state = ref.state();
		auto args = rawrbox::LuaUtils::getStringVariadicArgs(state);
		if (args.empty() || args.size() < 2) throw std::runtime_error("Missing params");

		std::string id = args[0];
		std::string key = args[1];

		if (id.empty()) {
			lua_getfield(state, LUA_ENVIRONINDEX, "__mod_id");
			if (lua_type(state, -1) != LUA_TSTRING) throw std::runtime_error("Invalid mod! Missing '__mod_id' on lua env");

			id = lua_tostring(state, -1);
		}

		return rawrbox::I18N::get(id, key, {args.begin() + 2, args.end()});
	}

	void I18NWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("i18n")
		    // UTILS ---
		    .addFunction("getLanguage", &rawrbox::I18NWrapper::getLanguage)
		    .addFunction("setLanguage", &rawrbox::I18NWrapper::setLanguage)
		    // --------
		    .addFunction("get", &rawrbox::I18NWrapper::get)
		    .endNamespace();
	}
} // namespace rawrbox
