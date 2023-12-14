#pragma once

#include <rawrbox/scripting/hooks.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/utils/console.hpp>
#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/file_watcher.hpp>
#include <rawrbox/utils/string.hpp>

#include <sol/sol.hpp>

#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace rawrbox {

	template <typename T>
	concept isLuaType = requires(sol::state lua) {
		{ T::registerLua(lua) };
	};

	class SCRIPTING {
	protected:
		static std::unordered_map<std::string, std::unique_ptr<rawrbox::Mod>> _mods;
		static std::unordered_map<std::string, std::vector<std::string>> _loadedLuaFiles;

		static std::unique_ptr<rawrbox::FileWatcher> _watcher;
		static std::unique_ptr<rawrbox::Hooks> _hooks;
		static std::unique_ptr<sol::state> _lua;

		static std::vector<std::unique_ptr<rawrbox::Plugin>> _plugins;

		static bool _hotReloadEnabled;
		static rawrbox::Console* _console;

		// LOAD ----
		static void loadLibraries();
		static void loadTypes();
		static void loadLuaExtensions(rawrbox::Mod* mod);
		static void loadGlobals(rawrbox::Mod* mod);
		static void loadI18N(rawrbox::Mod* mod);
		// ----

		static void registerLoadedFile(const std::string& modId, const std::string& filePath);
		static void hotReload(const std::string& filePath);

	public:
		static bool initialized;

		static void init(int hotReloadMs = 0);
		static void shutdown();

		static void setConsole(rawrbox::Console* console);

		// EVENTS ----
		static rawrbox::Event<> onRegisterTypes;
		static rawrbox::Event<rawrbox::Mod*> onRegisterGlobals;
		static rawrbox::Event<rawrbox::Mod*> onLoadExtensions;
		static rawrbox::Event<rawrbox::Mod*> onModHotReload;
		// -------

		// PLUGINS ---
		template <typename T = rawrbox::Plugin, typename... CallbackArgs>
		static void registerPlugin(CallbackArgs&&... args) {
			auto plugin = std::make_unique<T>(std::forward<CallbackArgs>(args)...);

			fmt::print("[RawrBox-Scripting] Registered lua plugin '{}'\n", rawrbox::StrUtils::replace(typeid(T).name(), "class rawrbox::", ""));
			_plugins.push_back(std::move(plugin));
		}
		// -----

		// LOAD -----
		static void load();
		static bool loadLuaFile(const std::string& path, const sol::environment& env);
		[[nodiscard]] static bool isLuaFileMounted(const std::string& path);
		//------

		// UTILS -----
		[[nodiscard]] static sol::state& getLUA();
		[[nodiscard]] static const std::unordered_map<std::string, std::unique_ptr<Mod>>& getMods();
		[[nodiscard]] static const std::vector<std::string> getModsIds();

		[[nodiscard]] static bool hotReloadEnabled();

		template <typename T>
		static void registerType() {
			if (_lua == nullptr) throw std::runtime_error("[RawrBox-Scripting] LUA is not set! Reference got destroyed?");

			if constexpr (isLuaType<T>) {
				T::registerLua(*_lua);
			} else {
				throw std::runtime_error("[RawrBox-Scripting] Type missing 'registerLua'");
			}
		}

		template <typename... CallbackArgs>
		static void call(const std::string& hookName, CallbackArgs&&... args) {
			for (auto& mod : _mods) {
				mod.second->call(hookName, args...);
			}
		}
		//------
	};
} // namespace rawrbox
