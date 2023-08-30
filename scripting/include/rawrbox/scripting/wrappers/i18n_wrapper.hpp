#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class I18NWrapper {
	public:
		[[nodiscard]] const std::string& getLanguage() const;
		void setLanguage(const std::string& language);

		std::string get(std::string id, const std::string& key, sol::this_environment modEnv, sol::variadic_args va);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
