#pragma once
#include <rawrbox/scripting/utils/lua.hpp>

#include <sol/sol.hpp>

#include <filesystem>
#include <string>

namespace rawrbox {
	class Scripting;

	class Mod {
	protected:
		sol::environment _environment;
		sol::table _modTable;

		std::filesystem::path _folder;
		std::string _id;

	public:
		std::function<void(std::filesystem::path)> onLUAReload = nullptr;

		Mod(std::string id, std::filesystem::path folderName);
		Mod(const Mod&) = default;
		Mod(Mod&&) = delete;
		Mod& operator=(const Mod&) = default;
		Mod& operator=(Mod&&) = delete;
		virtual ~Mod();

		virtual void init();
		virtual bool load();
		virtual void preLoad();

		// UTILS ----
		[[nodiscard]] virtual const std::string& getID() const;
		[[nodiscard]] virtual const std::string getEntryFilePath() const;
		[[nodiscard]] virtual const std::filesystem::path& getFolder() const;

		virtual sol::environment& getEnvironment();
		// -----

		template <typename... CallbackArgs>
		void call(const std::string& name, CallbackArgs&&... args) {
			sol::function func = this->_modTable[name];
			if (func.get_type() != sol::type::function) return;

			rawrbox::LuaUtils::runCallback(func, this->_modTable, std::forward<CallbackArgs>(args)...);
		}
	};
} // namespace rawrbox
