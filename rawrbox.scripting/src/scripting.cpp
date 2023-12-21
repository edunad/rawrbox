#include <rawrbox/engine/static.hpp>
#include <rawrbox/scripting/scripting.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/console_command_wrapper.hpp>
#include <rawrbox/scripting/wrappers/console_wrapper.hpp>
#include <rawrbox/scripting/wrappers/fmt_wrapper.hpp>
#include <rawrbox/scripting/wrappers/hooks_wrapper.hpp>
#include <rawrbox/scripting/wrappers/i18n_wrapper.hpp>
#include <rawrbox/scripting/wrappers/io_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/aabb_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/bbox_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/color_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/matrix_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector2_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector3_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector4_wrapper.hpp>
#include <rawrbox/scripting/wrappers/mod_wrapper.hpp>
#include <rawrbox/scripting/wrappers/scripting_wrapper.hpp>
#include <rawrbox/scripting/wrappers/timer_wrapper.hpp>
#include <rawrbox/utils/i18n.hpp>
#include <rawrbox/utils/time.hpp>

#include <filesystem>
#include <string>

namespace rawrbox {
	// PROTECTED -----
	std::unordered_map<std::string, std::unique_ptr<rawrbox::Mod>> SCRIPTING::_mods = {};
	std::unordered_map<std::string, std::vector<std::string>> SCRIPTING::_loadedLuaFiles = {};

	std::unique_ptr<rawrbox::FileWatcher> SCRIPTING::_watcher = nullptr;
	std::unique_ptr<rawrbox::Hooks> SCRIPTING::_hooks = std::make_unique<rawrbox::Hooks>();
	std::unique_ptr<sol::state> SCRIPTING::_lua = std::make_unique<sol::state>();

	std::vector<std::unique_ptr<rawrbox::Plugin>> SCRIPTING::_plugins = {};

	bool SCRIPTING::_hotReloadEnabled = false;
	rawrbox::Console* SCRIPTING::_console = nullptr;
	// -----

	// PUBLIC ----
	rawrbox::Event<> SCRIPTING::onRegisterTypes;
	rawrbox::Event<rawrbox::Mod*> SCRIPTING::onRegisterGlobals;
	rawrbox::Event<rawrbox::Mod*> SCRIPTING::onLoadExtensions;
	rawrbox::Event<rawrbox::Mod*> SCRIPTING::onModHotReload;

	bool SCRIPTING::initialized = false;
	// ------

	void SCRIPTING::init(int hotReloadMs) {
		_hotReloadEnabled = hotReloadMs > 0;
		if (hotReloadEnabled()) {
			fmt::print("[RawrBox-Scripting] Enabled lua hot-reloading\n  └── Delay: {}ms\n", hotReloadMs);

			_watcher = std::make_unique<rawrbox::FileWatcher>(
			    [](std::string pth, rawrbox::FileStatus status) {
				    if (status != rawrbox::FileStatus::modified) return;
				    hotReload(pth);
			    },
			    std::chrono::milliseconds(hotReloadMs));
			_watcher->start();
		}

		_mods.clear();

		// Loading initial libs ---
		loadLibraries();
		loadTypes();
		//  ----

		initialized = true;
	}

	void SCRIPTING::shutdown() {
		for (auto& mod : _mods) {
			auto& env = mod.second->getEnvironment();
			env.clear();
		}

		_watcher.reset();
		_hooks.reset();

		_mods.clear();
		_plugins.clear();

		_lua->collect_garbage();
		_lua.reset();
	}

	void SCRIPTING::setConsole(rawrbox::Console* console) {
		_console = console;
	}

	// LOAD ---
	void SCRIPTING::load() {
		if (!std::filesystem::exists("./mods")) throw std::runtime_error("[RawrBox-Scripting] Failed to locate folder './mods'");

		// TODO: Do we need mod load ordering?
		for (auto& p : std::filesystem::directory_iterator("./mods")) {
			if (!p.is_directory()) continue;

			auto id = p.path().filename().string();
			auto folderPath = fmt::format("mods/{}", id);

			// Done
			auto mod = std::make_unique<rawrbox::Mod>(id, folderPath);
			_mods.emplace(id, std::move(mod));
		}

		// Initialize
		for (auto& mod : _mods) {
			mod.second->init();

			loadLuaExtensions(mod.second.get());
			loadGlobals(mod.second.get());
			loadI18N(mod.second.get());

			if (!mod.second->load()) {
				fmt::print("[RawrBox-Scripting] Failed to load mod '{}'\n", mod.first);
			} else {
				// Register file for hot-reloading
				registerLoadedFile(mod.first, mod.second->getEntryFilePath());
				// ----
			}
		}
		// -----
	}

	void SCRIPTING::loadGlobals(rawrbox::Mod* mod) {
		if (mod == nullptr) return;

		auto& env = mod->getEnvironment();
		env["print"] = [](sol::variadic_args va) {
			auto vars = std::vector<sol::object>(va.begin(), va.end());

			std::vector<std::string> prtData;
			for (auto& var : vars) {
				prtData.push_back((*_lua)["tostring"](var));
			}

			if (prtData.empty()) return;
			fmt::print("{}\n", fmt::join(prtData, " "));
		};

		env["printTable"] = [](sol::table table) {
			auto json = rawrbox::LuaUtils::luaToJsonObject(table);
			fmt::print("{}\n", json.dump(1, ' ', false));
		};

		env["include"] = [&env, mod](const std::string& path) {
			auto fixedPath = LuaUtils::getContent(path, mod->getFolder());

			bool loaded = loadLuaFile(fixedPath, env);
			if (!loaded) fmt::print("[RawrBox-Scripting] Failed to load '{}'\n", fixedPath);

			// Register file for hot-reloading
			registerLoadedFile(mod->getID(), fixedPath);
			// ----

			return loaded ? 1 : 0;
		};

		// Security ------------------------------------
		env["debug"]["setlocal"] = []() {
			fmt::print("[RawrBox-Scripting] 'debug.setlocal' removed due to security reasons\n");
			return;
		};

		env["debug"]["setupvalue"] = []() {
			fmt::print("[RawrBox-Scripting] 'debug.setupvalue' removed due to security reasons\n");
			return;
		};

		env["debug"]["upvalueid"] = []() {
			fmt::print("[RawrBox-Scripting] 'debug.upvalueid' removed due to security reasons\n");
			return;
		};

		env["debug"]["upvaluejoin"] = []() {
			fmt::print("[RawrBox-Scripting] 'debug.upvaluejoin' removed due to security reasons\n");
			return;
		};
		// --------------

		// Default utils -----
		env["curtime"] = []() { return rawrbox::TimeUtils::curtime(); };
		env["time"] = []() { return rawrbox::TimeUtils::time(); };

		env["DELTA_TIME"] = []() { return rawrbox::DELTA_TIME; };
		env["FIXED_DELTA_TIME"] = []() { return rawrbox::FIXED_DELTA_TIME; };
		env["FRAME_ALPHA"] = []() { return rawrbox::FRAME_ALPHA; };
		// -------------------

		// Global types ------------------------------------
		env["fmt"] = rawrbox::FMTWrapper();
		env["io"] = rawrbox::IOWrapper();
		env["hooks"] = rawrbox::HooksWrapper(_hooks.get());
		env["scripting"] = rawrbox::ScriptingWrapper();
		env["timer"] = rawrbox::TimerWrapper();
		env["i18n"] = rawrbox::I18NWrapper();
		if (_console != nullptr) {
			env["console"] = rawrbox::ConsoleWrapper(_console);
		}
		// -------------------

		// ID ------------------
		env["__mod_folder"] = mod->getFolder().generic_string();
		env["__mod_id"] = mod->getID();
		// ---------------------

		// Register plugins env types ---
		for (auto& p : _plugins)
			p->registerGlobal(mod);
		//  -----

		// Custom global env types ---
		onRegisterGlobals(mod);
		// ----
	}

	void SCRIPTING::loadI18N(rawrbox::Mod* mod) {
		auto i18nPath = fmt::format("{}/i18n", mod->getFolder().generic_string());
		if (!std::filesystem::exists(i18nPath)) return;

		rawrbox::I18N::loadLanguagePack(mod->getID(), i18nPath);
	}

	void SCRIPTING::registerLoadedFile(const std::string& modId, const std::string& filePath) {
		auto mdFnd = _loadedLuaFiles.find(modId);
		if (mdFnd != _loadedLuaFiles.end()) {
			auto fileFnd = std::find(mdFnd->second.begin(), mdFnd->second.end(), filePath);
			if (fileFnd != mdFnd->second.end()) return; // Already registered
			mdFnd->second.push_back(filePath);
		} else {
			_loadedLuaFiles[modId] = {filePath};
		}

		if (hotReloadEnabled()) {
			_watcher->watchFile(filePath);
		}
	}

	void SCRIPTING::hotReload(const std::string& filePath) {
		// Find the owner
		for (auto& pt : _loadedLuaFiles) {
			auto fnd = std::find(pt.second.begin(), pt.second.end(), filePath) != pt.second.end();
			if (!fnd) continue;

			auto md = _mods.find(pt.first);
			if (md == _mods.end()) return;

			fmt::print("[RawrBox-Scripting] Hot-reloading lua file '{}'\n", filePath);
			getLUA().collect_garbage();
			loadLuaFile(filePath, md->second->getEnvironment());
			onModHotReload(md->second.get());
			break;
		};
	}

	// LOAD -----
	void SCRIPTING::loadLibraries() {
		if (_lua == nullptr) throw std::runtime_error("[RawrBox-Scripting] LUA is not set! Reference got destroyed?");

		_lua->open_libraries(sol::lib::base);
		_lua->open_libraries(sol::lib::package);
		_lua->open_libraries(sol::lib::math);
		_lua->open_libraries(sol::lib::table);
		_lua->open_libraries(sol::lib::debug);
		_lua->open_libraries(sol::lib::string);
		_lua->open_libraries(sol::lib::coroutine);
		_lua->open_libraries(sol::lib::bit32);

#ifdef RAWRBOX_SCRIPTING_LUAJIT
		_lua->open_libraries(sol::lib::jit);
#endif
	}

	void SCRIPTING::loadTypes() {
		if (_lua == nullptr) throw std::runtime_error("[RawrBox-Scripting] LUA is not set! Reference got destroyed?");

		// Math ----
		rawrbox::AABBWrapper::registerLua(*_lua);
		rawrbox::BBOXWrapper::registerLua(*_lua);
		rawrbox::ColorWrapper::registerLua(*_lua);
		rawrbox::MatrixWrapper::registerLua(*_lua);
		rawrbox::Vector2Wrapper::registerLua(*_lua);
		rawrbox::Vector3Wrapper::registerLua(*_lua);
		rawrbox::Vector4Wrapper::registerLua(*_lua);
		// ----

		// Default ----
		rawrbox::IOWrapper::registerLua(*_lua);
		rawrbox::FMTWrapper::registerLua(*_lua);
		rawrbox::ScriptingWrapper::registerLua(*_lua);
		rawrbox::ModWrapper::registerLua(*_lua);
		rawrbox::HooksWrapper::registerLua(*_lua);
		rawrbox::TimerWrapper::registerLua(*_lua);
		rawrbox::I18NWrapper::registerLua(*_lua);

		if (_console != nullptr) {
			rawrbox::ConsoleCommandWrapper::registerLua(*_lua);
			rawrbox::ConsoleWrapper::registerLua(*_lua);
		}
		// ----

		// Register plugins types ---
		for (auto& p : _plugins)
			p->registerTypes(*_lua);
		//  -----

		// Custom ----
		onRegisterTypes();
		// ----
	}

	void SCRIPTING::loadLuaExtensions(rawrbox::Mod* mod) {
		if (_lua == nullptr) throw std::runtime_error("[RawrBox-Scripting] LUA is not set! Reference got destroyed?");
		if (mod == nullptr) return;

		auto& env = mod->getEnvironment();
		loadLuaFile("./lua/table.lua", env);
		loadLuaFile("./lua/string.lua", env);
		loadLuaFile("./lua/math.lua", env);
		loadLuaFile("./lua/json.lua", env);
		loadLuaFile("./lua/sha2.lua", env);
		loadLuaFile("./lua/util.lua", env);
		loadLuaFile("./lua/input.lua", env);

		if (_console != nullptr) {
			loadLuaFile("./lua/console_enums.lua", env);
		}

		// Register plugins types ---
		for (auto& p : _plugins)
			p->loadLuaExtensions(mod);
		//  -----

		// Custom ----
		onLoadExtensions(mod);
		// ----
	}

	bool SCRIPTING::loadLuaFile(const std::string& path, const sol::environment& env) {
		if (!std::filesystem::exists(path)) {
			fmt::print("[RawrBox-Scripting] Failed to load lua : {}\n", path);
			return false;
		}

		std::string errStr;
		// try to load the file while handling exceptions
		auto ret = _lua->safe_script_file(
		    path, env, [&errStr](lua_State*, sol::protected_function_result pfr) {
			    sol::error err = pfr;
			    errStr = err.what();
			    return pfr;
		    },
		    sol::load_mode::text);

		// check if we loaded the file
		if (errStr.empty()) return true;

		fmt::print("[RawrBox-Scripting] Failed to load '{}'\n  └── Lua error : {}\n", path, errStr);
		return false;
	}

	bool SCRIPTING::isLuaFileMounted(const std::string& path) {
		for (auto& pt : _loadedLuaFiles) {
			auto fnd = std::find(pt.second.begin(), pt.second.end(), path) != pt.second.end();
			if (fnd) return true;
		};

		return false;
	}
	//  -----

	// UTILS -----
	sol::state& SCRIPTING::getLUA() { return *_lua; }

	const std::unordered_map<std::string, std::unique_ptr<Mod>>& SCRIPTING::getMods() {
		return _mods;
	}

	const std::vector<std::string> SCRIPTING::getModsIds() {
		std::vector<std::string> modNames = {};
		for (auto& mod : _mods) {
			modNames.push_back(mod.second->getID());
		}

		return modNames;
	}

	bool SCRIPTING::hotReloadEnabled() {
		return _hotReloadEnabled;
	}
	// -----

} // namespace rawrbox