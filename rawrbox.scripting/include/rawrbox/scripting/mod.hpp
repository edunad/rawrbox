
#pragma once

#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/logger.hpp>

#include <filesystem>
#include <memory>

namespace rawrbox {
	class Mod {
		// LUA ----
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

		// OTHER ----
		glz::json_t _metadata = {};
		// --------
	public:
		Mod(std::string id, std::filesystem::path folderPath, glz::json_t metadata);
		Mod(const Mod&) = delete;
		Mod(Mod&&) = delete;
		Mod& operator=(const Mod&) = delete;
		Mod& operator=(Mod&&) = delete;
		virtual ~Mod();

		virtual void init();
		virtual void shutdown();

		virtual void gc();

		// LOADING -------
		virtual void load();
		// ---------------

		virtual void script(const std::string& script);

		// UTILS ----
		[[nodiscard]] virtual const std::string& getID() const;

		[[nodiscard]] virtual std::string getEntryFilePath() const;
		[[nodiscard]] virtual const std::filesystem::path& getFolder() const;

		[[nodiscard]] virtual const glz::json_t& getMetadata() const;

		[[nodiscard]] virtual lua_State* getEnvironment();
		// -----

		template <typename... CallbackArgs>
		std::optional<luabridge::LuaResult> call(const std::string& name, CallbackArgs&&... args) {
			auto fnc = this->_modTable[name];
			if (!fnc.isCallable()) return std::nullopt;

			try {
				luabridge::LuaResult result = luabridge::call(fnc, this->_modTable, std::forward<CallbackArgs>(args)...);
				if (result.hasFailed()) _logger->warn("Lua error on {}\n  └── {}", this->_id, result.errorMessage());

				return result;
			} catch (const luabridge::LuaException& err) {
				_logger->warn("Lua error on {}\n  └── {}", this->_id, err.what());
				return std::nullopt;
			}
		}
	};
} // namespace rawrbox
