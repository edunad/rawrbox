#pragma once

#include <rawrbox/scripting/hooks.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/file_watcher.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/string.hpp>

namespace rawrbox {

	template <typename T>
	concept isLuaType = requires(lua_State* lua) {
		{ T::registerLua(lua) };
	};

	class SCRIPTING {
	protected:
		static std::unordered_map<std::string, std::unique_ptr<rawrbox::Mod>> _mods;
		static std::unordered_map<std::string, std::vector<std::string>> _loadedLuaFiles;

		static std::vector<std::unique_ptr<rawrbox::ScriptingPlugin>> _plugins;
		static std::unique_ptr<rawrbox::FileWatcher> _watcher;
		static std::unique_ptr<rawrbox::Hooks> _hooks;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		// LUA ----
		static lua_State* _L;
		// ----------

		static bool _hotReloadEnabled;

		// MODS ---
		static void prepareMods();
		// --------

		// LOAD ----
		static void loadLibraries();
		static void loadTypes();
		static void loadGlobals();
		// ----

		// HOT RELOAD ---
		static void registerLoadedFile(const std::string& modId, const std::string& filePath);
		static void hotReload(const std::string& filePath);
		// -------------

	public:
		static bool initialized;

		// EVENTS ----
		static rawrbox::Event<> onRegisterTypes;
		static rawrbox::Event<> onRegisterGlobals;
		static rawrbox::Event<> onLoadLibraries;
		static rawrbox::Event<rawrbox::Mod*> onModHotReload;
		// -------

		// PLUGINS ---
		template <typename T = rawrbox::ScriptingPlugin, typename... CallbackArgs>
		static void registerPlugin(CallbackArgs&&... args) {
			auto plugin = std::make_unique<T>(std::forward<CallbackArgs>(args)...);

			_logger->info("Registered lua plugin '{}'", rawrbox::StrUtils::replace(typeid(T).name(), "class rawrbox::", ""));
			_plugins.push_back(std::move(plugin));
		}
		// -----

		template <typename T>
		static void registerType() {
			if (_L == nullptr) throw _logger->error("LUA is not set! Reference got destroyed?");

			if constexpr (isLuaType<T>) {
				T::registerLua(*_L);
			} else {
				throw _logger->error("[RawrBox-Scripting] Type missing 'registerLua'");
			}
		}

		template <typename... CallbackArgs>
		static void call(const std::string& hookName, CallbackArgs&&... args) {
			for (auto& mod : _mods) {
				mod.second->call(hookName, std::forward<CallbackArgs>(args)...);
			}
		}

		static void init(int hotReloadMs = 0);
		static void load();
		static void shutdown();

		// UTILS ---
		[[nodiscard]] static lua_State* getLUA();
		[[nodiscard]] static const std::unordered_map<std::string, std::unique_ptr<Mod>>& getMods();
		[[nodiscard]] static const std::vector<std::string> getModsIds();

		[[nodiscard]] static bool hotReloadEnabled();
		// ---------
	};
} // namespace rawrbox
