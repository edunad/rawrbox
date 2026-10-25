
#include <rawrbox/scripting/manager.hpp>
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
		auto* state = ref.state();
		auto args = rawrbox::LuaUtils::argsToString(state, true);
		if (args.empty() || args.size() < 2) throw std::runtime_error("Missing params");

		std::string id = args[0];
		std::string key = args[1];

		if (id.empty()) id = rawrbox::LuaUtils::getLuaENVVar<std::string>(state, "__mod_id");

		return rawrbox::I18N::get(id, key, {args.begin() + 2, args.end()});
	}

	std::string I18NWrapper::resolve(const std::string& text, lua_State* L) {
		const auto* mod = rawrbox::SCRIPTING::getMod(L);
		return rawrbox::I18N::resolve(text, mod == nullptr ? std::string_view() : std::string_view(mod->getID()));
	}

	void I18NWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("i18n", {})
		    // UTILS ---
		    .addFunction("getLanguage", &rawrbox::I18NWrapper::getLanguage)
		    .addFunction("setLanguage", &rawrbox::I18NWrapper::setLanguage)
		    // --------
		    .addFunction("get", &rawrbox::I18NWrapper::get)
		    .addFunction("resolve", &rawrbox::I18NWrapper::resolve)
		    .endNamespace();
	}
} // namespace rawrbox
