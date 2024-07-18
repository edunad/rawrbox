#pragma once

#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/utils/console.hpp>
#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/file_watcher.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/string.hpp>

namespace rawrbox {
	class SCRIPTING {
	protected:
		static std::unordered_map<std::string, std::unique_ptr<rawrbox::Mod>> _mods;
		static std::unordered_map<std::string, std::vector<std::filesystem::path>> _loadedLuaFiles;

		static std::vector<std::unique_ptr<rawrbox::ScriptingPlugin>> _plugins;
		static std::unique_ptr<rawrbox::FileWatcher> _watcher;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static rawrbox::Console* _console;
		static bool _hotReloadEnabled;

		// LOAD ----
		static void loadLibraries(rawrbox::Mod& mod);
		static void loadTypes(rawrbox::Mod& mod);
		static void loadGlobals(rawrbox::Mod& mod);
		// ----

		// MOD LOAD ---
		static void loadI18N(const rawrbox::Mod& mod);
		// ------------

		// HOT RELOAD ---
		static void registerLoadedFile(const std::string& modId, const std::filesystem::path& filePath);
		static void hotReload(const std::filesystem::path& filePath);
		// -------------

	public:
		static bool initialized;

		// EVENTS ----
		static rawrbox::Event<rawrbox::Mod&> onRegisterTypes;
		static rawrbox::Event<rawrbox::Mod&> onRegisterGlobals;
		static rawrbox::Event<rawrbox::Mod&> onLoadLibraries;
		static rawrbox::Event<rawrbox::Mod&> onModHotReload;
		// -------

		// PLUGINS ---
		template <typename T = rawrbox::ScriptingPlugin, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::ScriptingPlugin>)
		static void registerPlugin(CallbackArgs&&... args) {
			auto plugin = std::make_unique<T>(std::forward<CallbackArgs>(args)...);
			auto pluginName = rawrbox::StrUtils::replace(typeid(T).name(), "class ", "");

			_logger->info("Registered lua plugin '{}'", fmt::styled(pluginName, fmt::fg(fmt::color::coral)));
			_plugins.push_back(std::move(plugin));
		}
		// -----

		template <typename... CallbackArgs>
		static void call(const std::string& hookName, CallbackArgs&&... args) {
			for (auto& mod : _mods) {
				mod.second->call(hookName, std::forward<CallbackArgs>(args)...);
			}
		}

		static void init(int hotReloadMs = 0);

		// LOADING ----
		static std::unordered_map<std::filesystem::path, bool> loadMods(const std::filesystem::path& rootFolder);
		static bool loadMod(const std::filesystem::path& modFolder);

		static bool unloadMod(const std::filesystem::path& modFolder);
		static bool unloadMod(const std::string& modId);
		// -----------

		static void shutdown();

		static void setConsole(rawrbox::Console* console);

		// UTILS ---
		[[nodiscard]] static const std::unordered_map<std::string, std::unique_ptr<rawrbox::Mod>>& getMods();
		[[nodiscard]] static std::vector<std::string> getModsIds();
		[[nodiscard]] static bool hotReloadEnabled();
		[[nodiscard]] static bool isLuaFileMounted(const std::string& path);
		// ---------
	};
} // namespace rawrbox
