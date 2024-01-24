#include <rawrbox/engine/static.hpp>
#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/time.hpp>

#include <fmt/format.h>

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

	// LUA ------
	lua_State* SCRIPTING::_L = nullptr;
	// -------------

	// --------------

	// PUBLIC ----
	bool SCRIPTING::initialized = false;
	// ------

	// LOAD -----
	void SCRIPTING::loadLibraries() {
		if (_L == nullptr) throw _logger->error("LUA is not set! Reference got destroyed?");

		// COMMON -----
		luaL_openlibs(_L); // Should be safe, since LUAU takes care of non-secure libs (https://luau-lang.org/sandbox#library)
				   //  -----
	}

	void SCRIPTING::loadTypes() {
		if (_L == nullptr) throw _logger->error("LUA is not set! Reference got destroyed?");
		// Register types, these will be read-only & sandboxed!
	}

	void SCRIPTING::loadGlobals() {
		if (_L == nullptr) throw _logger->error("LUA is not set! Reference got destroyed?");
		// Register globals, these will be read-only & sandboxed!

		auto globalTable = luabridge::getGlobalNamespace(_L);

		// TIME UTILS ---
		globalTable.addFunction("curtime", []() { return rawrbox::TimeUtils::curtime(); });
		globalTable.addFunction("time", []() { return rawrbox::TimeUtils::time(); });
		// -------------

		// UTILS ----
		globalTable.addFunction("printTable", [](luabridge::LuaRef ref) {
			nlohmann::json json = rawrbox::LuaUtils::luaToJsonObject(ref);
			fmt::print("{}\n", json.dump(1, ' ', false));
		});
		// ----------

		// ENGINE ------
		auto engine = globalTable.beginNamespace("engine");
		engine.addVariable("deltaTime", rawrbox::DELTA_TIME);
		engine.addVariable("fixedDeltaTime", rawrbox::FIXED_DELTA_TIME);
		engine.addVariable("frameAlpha", rawrbox::FRAME_ALPHA);
		engine.endNamespace();
		// -------------
	}
	// ----------

	void SCRIPTING::init(int hotReloadMs) {
		_L = luaL_newstate();

		_hotReloadEnabled = hotReloadMs > 0;
		if (_hotReloadEnabled) {
			/*fmt::print("[RawrBox-Scripting] Enabled lua hot-reloading\n  └── Delay: {}ms\n", hotReloadMs);

			_watcher = std::make_unique<rawrbox::FileWatcher>(
			    [](std::string pth, rawrbox::FileStatus status) {
				    if (status != rawrbox::FileStatus::modified) return;
				    hotReload(pth);
			    },
			    std::chrono::milliseconds(hotReloadMs));
			_watcher->start();*/
		}

		_mods.clear();

		// Loading initial libs ---
		loadLibraries();
		loadTypes();
		loadGlobals();
		//  ----

		// Freeze lua env ---
		luaL_sandbox(_L);
		// --------------

		rawrbox::Mod test = {_L, "test", "./assets/mods/test-luau"};
		test.init();
		test.load();

		/*
				L = luaL_newstate();

				// Open default libs
				luaL_openlibs(L);
				//---------------

				// Register types ----
				auto globalTable = luabridge::getGlobalNamespace(L);
				globalTable.addFunction("curtime", []() { return 0; });
				// ----------------

				// Freeze lua env ---
				luaL_sandbox(L);
				// --------------

				_test = std::make_unique<Mod>(L, "my-mod", "./assets/mods/test-luau");
				_test->init();
				_test->load();

				_test->call("init");

				_test->call("update", "balblabla");

				initialized = true;

				// luaL_sandbox(L);
				// luaL_openlibs(L);

				// lua_setreadonly(L, LUA_GLOBALSINDEX, true);
				// lua_setsafeenv(L, LUA_GLOBALSINDEX, true);

				initialized = true;

				L = luaL_newstate();

				// lua_setreadonly(L, LUA_GLOBALSINDEX, true);
				lua_setsafeenv(L, LUA_GLOBALSINDEX, true);

				luaL_openlibs(L);

				// Setup namespaces ---
				luabridge::getGlobalNamespace(L)
				    .beginNamespace("test")
				    .addFunction("foo", []() {
					    fmt::print("aaaa\n");
				    })
				    .endNamespace();

				// Test MOD::Init()
				luabridge::getGlobalNamespace(L)
				    .beginNamespace("MOD")
				    .addFunction("init", []() {
					    fmt::print("aaaa\n");
				    })
				    .endNamespace();

				auto init = luabridge::getGlobal(L, "init");

				// ----------

				rawrbox::LuaScript test = {};
				test.compile("./assets/mods/luau_test.luau");
				test.load();
				*/
	}

	void SCRIPTING::load() {
		if (!std::filesystem::exists("./mods")) throw _logger->error("Failed to locate folder './mods'");

		// TODO: mod load ordering
		for (auto& p : std::filesystem::directory_iterator("./mods")) {
			if (!p.is_directory()) continue;

			auto id = p.path().filename().string();
			auto folderPath = fmt::format("mods/{}", id);

			// Done
			auto mod = std::make_unique<rawrbox::Mod>(_L, id, folderPath);
			_mods.emplace(id, std::move(mod));
		}

		// Initialize
		for (auto& mod : _mods) {
			mod.second->init();

			// loadLuaExtensions(mod.second.get());
			// loadGlobals(mod.second.get());
			// loadI18N(mod.second.get());

			try {
				mod.second->load();

				// Register file for hot-reloading
				// registerLoadedFile(mod.first, mod.second->getEntryFilePath());
				// ----
			} catch (const cpptrace::exception_with_message err) {
				_logger->warn("{}", err.message());
			}
		}
		// -----
	}

	void SCRIPTING::shutdown() {
		_L = nullptr;
	}

	// UTILS ----
	lua_State* SCRIPTING::getLUA() { return _L; }

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
