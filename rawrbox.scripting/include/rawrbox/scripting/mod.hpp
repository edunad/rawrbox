
#pragma once

#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/utils/logger.hpp>

#include <filesystem>
#include <memory>

namespace rawrbox {
	class Mod {
		// LUA ----
		lua_State* _LSandbox = nullptr;
		lua_State* _L = nullptr;
		// --------

		// TABLE ---
		luabridge::LuaRef _modTable;
		// ---------

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Mod");
		// -------------

		// Settings ---
		std::filesystem::path _folder;
		std::string _id = "UNKNOWN";
		// -----------

	public:
		Mod(lua_State* L, const std::string id, std::filesystem::path folderName);
		Mod(const Mod&) = delete;
		Mod(Mod&&) = delete;
		Mod& operator=(const Mod&) = delete;
		Mod& operator=(Mod&&) = delete;
		virtual ~Mod();

		virtual void init();

		// LOADING -------
		virtual void load();
		// ---------------

		// UTILS ----
		[[nodiscard]] virtual const std::string& getID() const;
		[[nodiscard]] virtual const std::string getEntryFilePath() const;
		[[nodiscard]] virtual const std::filesystem::path& getFolder() const;

		virtual lua_State* getEnvironment();
		// -----

		template <typename... CallbackArgs>
		void call(const std::string& name, CallbackArgs&&... args) {
			auto fnc = this->_modTable[name];
			if (!fnc.isCallable()) return;

			try {
				fnc(fnc, std::forward<CallbackArgs>(args)...);
			} catch (luabridge::LuaException& err) {
				throw _logger->error("{}", err.what());
			}
		}
	};
} // namespace rawrbox
