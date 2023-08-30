
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/i18n_wrapper.hpp>
#include <rawrbox/utils/i18n.hpp>

#include <fmt/format.h>

namespace rawrbox {
	const std::string& I18NWrapper::getLanguage() const {
		return rawrbox::I18N::getLanguage();
	}

	void I18NWrapper::setLanguage(const std::string& language) {
		return rawrbox::I18N::setLanguage(language);
	}

	std::string I18NWrapper::get(std::string id, const std::string& key, sol::this_environment modEnv, sol::variadic_args va) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-BASSWrapper] MOD not set!");
		auto vars = std::vector<sol::object>(va.begin(), va.end());
		std::vector<std::string> args;

		if (id.empty()) id = modEnv.env.value()["__mod_id"];
		for (auto& var : vars) {
			if (var.get_type() == sol::type::string) {
				args.push_back(var.as<std::string>());
			} else if (var.get_type() == sol::type::number) {
				if (var.is<int>()) args.push_back(std::to_string(var.as<int>()));
				if (var.is<double>()) args.push_back(std::to_string(var.as<double>()));
				if (var.is<float>()) args.push_back(std::to_string(var.as<float>()));
			}
		}

		return rawrbox::I18N::get(id, key, args);
	}

	void I18NWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<I18NWrapper>("i18n",
		    sol::no_constructor,

		    "getLanguage", &I18NWrapper::getLanguage,
		    "setLanguage", &I18NWrapper::setLanguage,

		    "get", &I18NWrapper::get);
	}
} // namespace rawrbox
