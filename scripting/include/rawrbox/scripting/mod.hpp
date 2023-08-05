#pragma once
#include <rawrbox/scripting/utils/lua.hpp>

#include <sol/sol.hpp>

#include <string>

namespace rawrbox {
	class Scripting;

	class Mod {
	protected:
		sol::environment _environment;
		sol::table _modTable;

		rawrbox::Scripting* _scripting = nullptr;

		std::string _folder;
		std::string _id;

	public:
		Mod(std::string id, std::string folderName);

		void init(Scripting& scripting_);
		bool load();
		void preLoad();

		// UTILS ----
		[[nodiscard]] const std::string& getID() const;
		[[nodiscard]] const std::string& getFolder() const;

		[[nodiscard]] const rawrbox::Scripting& getScripting() const;
		sol::environment& getEnvironment();
		// -----

		template <typename... CallbackArgs>
		void call(const std::string& name, CallbackArgs&&... args) {
			sol::function func = this->_modTable[name];
			if (func.get_type() != sol::type::function) return;

			rawrbox::LuaUtils::runCallback(func, this->_modTable, std::forward<CallbackArgs>(args)...);
		}
	};
} // namespace rawrbox
