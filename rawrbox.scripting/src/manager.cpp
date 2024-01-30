#include <rawrbox/engine/static.hpp>
#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/hooks.hpp>
#include <rawrbox/scripting/wrappers/i18n.hpp>
#include <rawrbox/scripting/wrappers/io.hpp>
#include <rawrbox/scripting/wrappers/mod.hpp>
#include <rawrbox/scripting/wrappers/timer.hpp>
#include <rawrbox/utils/i18n.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/time.hpp>

#include <fmt/format.h>

/*⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣠⣤⣤⣤⣄⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢺⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠋⠉⠉⠁⠀⠈⠛⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⢀⠀⠙⣿⠿⣿⣿⣿⣿⣿⣿⣿⣷⣶⣾⣿⣿⣿⣶⣤⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠻⠿⢿⣿⣿⣿⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠟⠛⠛⣛⡛⡿⣿⡟⢻⣿⣿⣿⣿⣿⣿⣣⣼⡟⠛⠳⣆⠘⢻⣿⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠋⠀⣶⠉⠉⠙⠶⠉⠁⣸⣿⣿⣿⣿⣿⣿⣯⣝⣀⣀⣀⣩⣄⣽⣿⣿⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⣶⣶⣶⣾⠟⠀⣰⣿⣿⣿⡿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⠿⣿⣿⣿⣾⠀⣼⣿⣿⣿⣿⣧⣈⡙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⣿⣿⣿⣏⠀⠹⢿⣿⣿⡿⠿⠿⢿⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣿⣿⠟⠀⠀⠈⠙⠛⢛⣃⣀⢀⠈⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀ ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⡿⠁⢠⡶⠿⠿⠿⠟⠻⠿⠛⠿⣷⣦⣹⣿⣿⣿⣿⣿⣿⣿⣿⡿⠀⠀⠀ ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣈⠿⣿⠀⣀⡈⠋⠀⢀⣀⣀⣀⣀⡀⠈⠹⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠀⠉⣰⡏⠀⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠛⠀⠾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡏⠁⠛⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢽⠛⠛⣿⣿⣿⡟⣿⣿⣿⣿⡇⠀⠀⠒⠀⢠⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠖⠃⡀⢿⡿⠃⢸⣿⣿⡅⠀⢀⣀⠀⠀⢫⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠛⠛⠃⠀⠘⢿⡿⠇⢸⡇⠀⠀⢄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣴⣦⣤⣄⣀⣤⣤⣾⣿⣿⣦⢻⠀⢀⣿⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠀⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣿⣿⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣶⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⣿⠿⠟⠋⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀

				   If you are stuck with lua, just remember...
							What would garry do?
*/

namespace rawrbox {
	// PROTECTED ----
	std::unordered_map<std::string, std::unique_ptr<rawrbox::Mod>> SCRIPTING::_mods = {};
	std::unordered_map<std::string, std::vector<std::string>> SCRIPTING::_loadedLuaFiles = {};

	std::unique_ptr<rawrbox::FileWatcher> SCRIPTING::_watcher = nullptr;
	std::vector<std::unique_ptr<rawrbox::ScriptingPlugin>> SCRIPTING::_plugins = {};

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> SCRIPTING::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SCRIPTING");
	// -------------

	bool SCRIPTING::_hotReloadEnabled = false;
	// --------------

	// PUBLIC ----
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onRegisterTypes;
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onRegisterGlobals;
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onLoadLibraries;
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onModHotReload;

	bool SCRIPTING::initialized = false;
	// ------

	// LOAD -----
	void SCRIPTING::loadLibraries(rawrbox::Mod& mod) {
		auto L = mod.getEnvironment();
		if (L == nullptr) throw _logger->error("LUA is not set! Reference got destroyed?");

		// COMMON -----
		luaL_openlibs(L); // Should be safe, since LUAU takes care of non-secure libs (https://luau-lang.org/sandbox#library)
				  //  -----

		// OTHER LIBS ---
		rawrbox::LuaUtils::compileAndLoad(L, "SHA", "./lua/sha2.lua");
		rawrbox::LuaUtils::compileAndLoad(L, "JSON", "./lua/json.lua");
		// --------------

		// Rawrbox LIBS ----
		rawrbox::LuaUtils::compileAndLoad(L, "RawrBox::Math", "./lua/math.lua");
		rawrbox::LuaUtils::compileAndLoad(L, "RawrBox::String", "./lua/string.lua");
		rawrbox::LuaUtils::compileAndLoad(L, "RawrBox::Table", "./lua/table.lua");
		// -----------------

		// Rawrbox enums ---
		// if (_console != nullptr) rawrbox::LuaUtils::compileAndLoad(_L, "RawrBox::Enums::Console", "./lua/enums/console.lua");
		rawrbox::LuaUtils::compileAndLoad(L, "RawrBox::Enums::Input", "./lua/enums/input.lua");
		// -----------------

		// Register plugins libraries ---
		for (auto& p : _plugins)
			p->loadLibraries(L);
		//  -----

		// Custom ----
		onLoadLibraries(mod);
		// ----
	}

	void SCRIPTING::loadTypes(rawrbox::Mod& mod) {
		auto L = mod.getEnvironment();
		if (L == nullptr) throw _logger->error("LUA is not set! Reference got destroyed?");
		// Register types, these will be read-only & sandboxed!

		// Rawrbox ---
		rawrbox::Hooks::registerLua(L);
		rawrbox::TimerWrapper::registerLua(L);
		rawrbox::I18NWrapper::registerLua(L);
		rawrbox::MODWrapper::registerLua(L);

#ifdef RAWRBOX_SCRIPTING_UNSAFE
		rawrbox::IOWrapper::registerLua(L); // TODO: Might have security vulnerabilities
#endif
		// -----------

		// Register plugins types ---
		for (auto& p : _plugins)
			p->registerTypes(L);
		//  -----

		// Custom ----
		onRegisterTypes(mod);
		// ----
	}

	void SCRIPTING::loadGlobals(rawrbox::Mod& mod) {
		auto L = mod.getEnvironment();
		if (L == nullptr) throw _logger->error("LUA is not set! Reference got destroyed?");
		// Register globals, these will be read-only & sandboxed!

		auto globalTable = luabridge::getGlobalNamespace(L);

		// TIME UTILS ---
		globalTable.addFunction("curtime", []() { return rawrbox::TimeUtils::curtime(); });
		globalTable.addFunction("time", []() { return rawrbox::TimeUtils::time(); });
		// -------------

		// UTILS ----
		globalTable.addFunction("printTable", [](lua_State* state) {
			nlohmann::json json = rawrbox::LuaUtils::luaToJsonObject(state);
			_logger->info("{}", json.dump(1, ' ', false));
		});

		// Override print to support fmt?
		globalTable.addFunction("print", [](lua_State* state) {
			auto args = rawrbox::LuaUtils::getStringVariadicArgs(state);
			if (args.empty()) return;

			_logger->info("{}", fmt::join(args, " "));
		});

		globalTable.addFunction("include", [](lua_State* state) {
			if (lua_type(state, 1) != LUA_TSTRING) throw std::runtime_error("Invalid param, string expected");
			auto path = lua_tostring(state, 1);

			lua_getfield(state, LUA_ENVIRONINDEX, "__mod_id");
			if (lua_type(state, -1) != LUA_TSTRING) throw std::runtime_error("Invalid mod! Missing '__mod_id' on lua env");
			auto modID = lua_tostring(state, -1);

			lua_getfield(state, LUA_ENVIRONINDEX, "__mod_folder");
			if (lua_type(state, -1) != LUA_TSTRING) throw std::runtime_error("Invalid mod! Missing '__mod_folder' on lua env");
			auto modFolder = lua_tostring(state, -1);

			auto fixedPath = LuaUtils::getContent(path, modFolder);
			rawrbox::LuaUtils::compileAndLoad(state, modID, fixedPath);

			// Register file for hot-reloading
			registerLoadedFile(modID, fixedPath);
			// ----
		});
		// ---------

		// MODDING ---
		globalTable.addFunction("hasMOD", [](const std::string& id) {
			return _mods.find(id) != _mods.end();
		});

		globalTable.addFunction("getMOD", [](const std::string& id) {
			auto fnd = _mods.find(id);
			if (fnd == _mods.end()) throw std::runtime_error(fmt::format("Mod {} not found", id));

			return rawrbox::MODWrapper(fnd->second.get());
		});
		// ----------

		// ENGINE ------
		globalTable.beginNamespace("engine")
		    .addProperty("deltaTime", &rawrbox::DELTA_TIME, false)
		    .addProperty("fixedDeltaTime", &rawrbox::FIXED_DELTA_TIME, false)
		    .addProperty("frameAlpha", &rawrbox::FRAME_ALPHA, false)
		    .endNamespace();
		// -------------

		// Register plugins globals ---
		for (auto& p : _plugins)
			p->registerGlobal(L);
		//  -----

		// Custom ----
		onRegisterGlobals(mod);
		// -------------
	}
	// -------------

	// MOD LOAD ---
	void SCRIPTING::loadI18N(const rawrbox::Mod& mod) {
		auto i18nPath = fmt::format("{}/i18n", mod.getFolder().generic_string());
		if (!std::filesystem::exists(i18nPath)) return;

		rawrbox::I18N::loadLanguagePack(mod.getID(), i18nPath);
	}
	// ------------

	// HOT RELOAD -----
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
			_logger->info("Registered {} -> {} for hot reload", modId, filePath);
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
			_logger->warn("Hot-reloading lua file '{}'", filePath);

			// Cleanup and load -----
			auto env = md->second->getEnvironment();
			md->second->gc(); // Cleanup

			try {
				rawrbox::LuaUtils::compileAndLoad(env, md->first, filePath);
			} catch (const std::exception& err) {
				_logger->printError("{}", err.what());
			}
			// ---------------

			onModHotReload(*md->second);
			break;
		};
	}
	// -------------
	// ----------

	void SCRIPTING::init(int hotReloadMs) {
		_hotReloadEnabled = hotReloadMs > 0;
		if (_hotReloadEnabled) {
			_logger->info("Enabled lua hot-reloading\n  └── Delay: {}ms", hotReloadMs);

			_watcher = std::make_unique<rawrbox::FileWatcher>(
			    [](std::string pth, rawrbox::FileStatus status) {
				    if (status != rawrbox::FileStatus::modified) return;
				    hotReload(pth);
			    },
			    std::chrono::milliseconds(hotReloadMs));
			_watcher->start();
		}

		// Prepare mods sandboxed env (but dont load mod's lua) ---
		prepareMods();
		// ----------------
	}

	void SCRIPTING::prepareMods() {
		if (!std::filesystem::exists("./mods")) throw _logger->error("Failed to locate folder './mods'"); // TODO: SUPPORT FOLDER CONFIGURATION

		// TODO: mod load ordering & mod settings to inject on sandboxed env
		for (auto& p : std::filesystem::directory_iterator("./mods")) {
			if (!p.is_directory()) continue;

			auto id = p.path().filename().string();
			auto folderPath = fmt::format("mods/{}", id);

			auto mod = std::make_unique<rawrbox::Mod>(id, folderPath);

			// Prepare env ----------
			loadLibraries(*mod);
			loadTypes(*mod);
			loadGlobals(*mod);
			loadI18N(*mod);
			// ----------

			_mods.emplace(id, std::move(mod));
		}
	}

	void SCRIPTING::load() {
		if (!std::filesystem::exists("./mods")) throw _logger->error("Failed to locate folder './mods'");

		// Load & initialize
		for (auto& mod : _mods) {
			try {
				mod.second->init(); // Sandbox env
				mod.second->load();
			} catch (std::runtime_error err) {
				_logger->printError("{}", err.what());
			}

			registerLoadedFile(mod.first, mod.second->getEntryFilePath()); // Register file for hot-reloading
		}
		// -----
	}

	void SCRIPTING::shutdown() {
		_watcher.reset();

		_loadedLuaFiles.clear();
		_mods.clear();
		_plugins.clear();
	}

	// UTILS ----
	const std::unordered_map<std::string, std::unique_ptr<Mod>>& SCRIPTING::getMods() { return _mods; }
	const std::vector<std::string> SCRIPTING::getModsIds() {
		std::vector<std::string> modNames = {};
		for (auto& mod : _mods) {
			modNames.push_back(mod.second->getID());
		}

		return modNames;
	}

	bool SCRIPTING::hotReloadEnabled() { return _hotReloadEnabled; }
	// -----
} // namespace rawrbox
