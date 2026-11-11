#pragma once

#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/scripting/utils/checker.hpp>
#include <rawrbox/utils/console.hpp>
#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/file_watcher.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/string.hpp>

#include <expected>
#include <string>
#include <vector>

namespace rawrbox {
	namespace ScriptingLibraries {
		const uint32_t NONE = 0;

		// LUA FILES ---
		const uint32_t SHA2 = 1U << 0U;
		const uint32_t JSON = 1U << 1U;
		const uint32_t MATH = 1U << 2U;
		const uint32_t STRING = 1U << 3U;
		const uint32_t TABLE = 1U << 4U;
		const uint32_t ENUMS = 1U << 5U;
		// -------------

		// GLOBALS ---
		const uint32_t HOOKS = 1U << 6U;
		const uint32_t TIMER = 1U << 7U;
		const uint32_t PROFILER = 1U << 8U;
		const uint32_t I18N = 1U << 9U;
		const uint32_t MOD = 1U << 10U;
		const uint32_t TIME = 1U << 11U;
		const uint32_t ENGINE = 1U << 12U;
		const uint32_t IO = 1U << 13U;
		const uint32_t CONSOLE = 1U << 14U;
		const uint32_t MATH_TYPES = 1U << 15U;
		// -------------

		const uint32_t ALL = 0xFFFFFFFFU;
	}; // namespace ScriptingLibraries

	struct ScriptingSettings {
		uint32_t hotReloadMs = 0; // 0 = disabled
		bool typeCheck = false;
		bool safeEnv = true; // Luau safeenv

		uint32_t libraries = rawrbox::ScriptingLibraries::ALL;

		std::vector<std::filesystem::path> definitions = {}; // Luau definition files, aka .d.lua
	};

	class SCRIPTING {
	protected:
		static std::vector<std::unique_ptr<rawrbox::Mod>> _mods;
		static std::unordered_map<std::string, std::vector<std::filesystem::path>> _loadedLuaFiles;

		static std::vector<std::unique_ptr<rawrbox::ScriptingPlugin>> _plugins;
		static std::unique_ptr<rawrbox::FileWatcher> _watcher;
		static std::unique_ptr<rawrbox::LuaChecker> _checker;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static rawrbox::Console* _console;
		static bool _hotReloadEnabled;
		static bool _safeEnv;
		static uint32_t _libraries;

		// LOAD ----
		static void loadLibraries(rawrbox::Mod& mod);
		static void loadTypes(rawrbox::Mod& mod);
		static void loadGlobals(rawrbox::Mod& mod);
		static void loadModifiers(rawrbox::Mod& mod);
		// ----

		// MOD LOAD ---
		static void loadI18N(const rawrbox::Mod& mod);
		[[nodiscard]] static bool rankBefore(const rawrbox::Mod& a, const rawrbox::Mod& b);
		// ------------

		// HOT RELOAD ---
		static void loadFile(rawrbox::Mod& mod, lua_State* L, const std::filesystem::path& filePath);
		static void hotReload(const std::filesystem::path& filePath);
		// -------------

	public:
		static bool initialized;

		// EVENTS ----
		static rawrbox::Event<rawrbox::Mod&> onRegisterTypes;
		static rawrbox::Event<rawrbox::Mod&> onRegisterGlobals;
		static rawrbox::Event<rawrbox::Mod&> onLoadLibraries;
		static rawrbox::Event<rawrbox::Mod&> onLoadModifiers;
		static rawrbox::Event<rawrbox::Mod&> onModHotReload;
		static rawrbox::Event<rawrbox::Mod&> onModUnload;
		// -------

		// PLUGINS ---
		template <typename T = rawrbox::ScriptingPlugin, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::ScriptingPlugin>)
		static void registerPlugin(CallbackArgs&&... args) {
			auto plugin = std::make_unique<T>(std::forward<CallbackArgs>(args)...);
			const auto pluginName = rawrbox::StrUtils::replace(typeid(T).name(), "class ", "");

			SCRIPTING::_logger->info("Registered lua plugin '{}'", fmt::styled(pluginName, fmt::fg(fmt::color::coral)));
			SCRIPTING::_plugins.push_back(std::move(plugin));
		}
		// -----

		template <typename... CallbackArgs>
		static void call(const std::string& hookName, CallbackArgs&&... args) {
			for (auto& mod : SCRIPTING::_mods) {
				mod->call(hookName, args...);
			}
		}

		static void init(const rawrbox::ScriptingSettings& settings = {});

		// LOADING ----
		static std::vector<rawrbox::Mod*> loadMods(const std::filesystem::path& rootFolder);
		static rawrbox::Mod* loadMod(const std::string& id, const std::filesystem::path& modFolder);
		static void registerLoadedFile(const std::string& modId, const std::filesystem::path& filePath);
		[[nodiscard]] static std::expected<rawrbox::ModMetadata, std::string> loadMetadata(const std::filesystem::path& modFolder);

		static bool unloadMod(const std::filesystem::path& modFolder);
		static bool unloadMod(const std::string& modId);
		// -----------

		static void shutdown();

		static void setConsole(rawrbox::Console* console);

		// UTILS ---
		[[nodiscard]] static rawrbox::Mod* getMod(const std::string& id);
		[[nodiscard]] static rawrbox::Mod* getMod(lua_State* L);
		[[nodiscard]] static bool hasMod(const std::string& id);

		[[nodiscard]] static const std::vector<std::unique_ptr<rawrbox::Mod>>& getMods();
		[[nodiscard]] static std::vector<std::string> getModsIds();
		[[nodiscard]] static bool hotReloadEnabled();
		[[nodiscard]] static bool isLuaFileMounted(const std::string& path);
		[[nodiscard]] static bool hasLibrary(uint32_t library);

#ifdef RAWRBOX_SCRIPTING_WORKSHOP_MODDING
		[[nodiscard]] static std::vector<rawrbox::Mod*> getModsByWorkshopType(uint32_t type);
		[[nodiscard]] static rawrbox::Mod* getModByWorkshopId(uint64_t id);
#endif
		// ---------
	};
} // namespace rawrbox