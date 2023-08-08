#include <rawrbox/engine/static.hpp>
#include <rawrbox/scripting/scripting.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/fmt_wrapper.hpp>
#include <rawrbox/scripting/wrappers/hooks_wrapper.hpp>
#include <rawrbox/scripting/wrappers/io_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/aabb_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/color_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/matrix_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector2_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector3_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector4_wrapper.hpp>
#include <rawrbox/scripting/wrappers/mod_wrapper.hpp>
#include <rawrbox/scripting/wrappers/scripting_wrapper.hpp>
#include <rawrbox/utils/time.hpp>

#include <filesystem>
#include <string>

namespace rawrbox {
	Scripting::Scripting(int hotReloadMs) : _hotReloadEnabled(hotReloadMs > 0) {
		this->_lua = std::make_unique<sol::state>();
		this->_hooks = std::make_unique<rawrbox::Hooks>();

		if (this->hotReloadEnabled()) {
			fmt::print("[RawrBox-Scripting] Enabled lua hot-reloading\n  └── Delay: {}ms\n", hotReloadMs);

			this->_watcher = std::make_unique<rawrbox::FileWatcher>(
			    [this](std::filesystem::path pth, rawrbox::FileStatus status) {
				    if (status != rawrbox::FileStatus::modified) return;
				    this->hotReload(pth.generic_string());
			    },
			    std::chrono::milliseconds(hotReloadMs));
			this->_watcher->start();
		}

		this->_mods.clear();

		// Loading initial libs ---
		// NOLINTBEGIN(clang-analyzer-optin.cplusplus.VirtualCall)
		this->loadLibraries();
		this->loadTypes();
		// NOLINTEND(clang-analyzer-optin.cplusplus.VirtualCall)
		//  ----
	}

	Scripting::~Scripting() {
		this->_watcher.reset();
		this->_hooks.reset();
		this->_mods.clear();

		this->_lua->collect_garbage();
		this->_lua.reset();
	}

	void Scripting::load() {
		if (!std::filesystem::exists("./mods")) throw std::runtime_error("[RawrBox-Scripting] Failed to locate folder './mods'");

		// TODO: Do we need mod load ordering?
		for (auto& p : std::filesystem::directory_iterator("./mods")) {
			if (!p.is_directory()) continue;

			auto id = p.path().filename().string();
			auto folderPath = fmt::format("mods/{}", id);

			// Done
			auto mod = std::make_unique<rawrbox::Mod>(id, folderPath);
			this->_mods.emplace(id, std::move(mod));
		}

		// Initialize
		for (auto& mod : this->_mods) {
			mod.second->init(*this);

			this->loadLuaExtensions(mod.second.get());
			this->loadGlobals(mod.second.get());

			if (!mod.second->load()) {
				fmt::print("[RawrBox-Scripting] Failed to load mod '{}'\n", mod.first);
			} else {
				// Register file for hot-reloading
				this->registerLoadedFile(mod.first, mod.second->getEntryFilePath());
				// ----
			}
		}
		// -----
	}

	void Scripting::loadGlobals(rawrbox::Mod* mod) {
		if (mod == nullptr) return;

		auto& env = mod->getEnvironment();
		env["print"] = [this](sol::variadic_args va) {
			auto vars = std::vector<sol::object>(va.begin(), va.end());

			std::vector<std::string> prtData;
			for (auto& var : vars) {
				prtData.push_back((*this->_lua)["tostring"](var));
			}

			if (prtData.empty()) return;
			fmt::print("{}\n", fmt::join(prtData, " "));
		};

		env["printTable"] = [](sol::table table) {
			auto json = rawrbox::LuaUtils::luaToJsonObject(table);
			fmt::print("{}\n", json.dump(1, ' ', false));
		};

		env["include"] = [&env, &mod, this](const std::string& path) {
			auto modFolder = mod->getFolder().generic_string();
			auto fixedPath = LuaUtils::getContent(path, modFolder);

			bool loaded = this->loadLuaFile(fixedPath, env, this->getLua());
			if (!loaded) fmt::print("[RawrBox-Scripting] Failed to load '{}'\n", fixedPath);

			// Register file for hot-reloading
			this->registerLoadedFile(mod->getID(), fixedPath);
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
		env["hooks"] = rawrbox::HooksWrapper(this->_hooks.get());
		env["scripting"] = rawrbox::ScriptingWrapper(this);
		// -------------------

		// Custom global types ---
		this->onRegisterGlobals(mod);
		// ----
	}

	void Scripting::registerLoadedFile(const std::string& modId, const std::string& filePath) {
		auto mdFnd = this->_loadedLuaFiles.find(modId);
		if (mdFnd != this->_loadedLuaFiles.end()) {
			auto fileFnd = std::find(mdFnd->second.begin(), mdFnd->second.end(), filePath);
			if (fileFnd != mdFnd->second.end()) return; // Already registered
			mdFnd->second.push_back(filePath);
		} else {
			this->_loadedLuaFiles[modId] = {filePath};
		}

		if (this->hotReloadEnabled()) {
			this->_watcher->watchFile(filePath);
		}
	}

	void Scripting::hotReload(const std::string& filePath) {
		// Find the owner
		for (auto& pt : this->_loadedLuaFiles) {
			auto fnd = std::find(pt.second.begin(), pt.second.end(), filePath) != pt.second.end();
			if (!fnd) continue;

			auto md = this->_mods.find(pt.first);
			if (md == this->_mods.end()) return;

			fmt::print("[RawrBox-Scripting] Hot-reloading lua file '{}'\n", filePath);
			this->loadLuaFile(filePath, md->second->getEnvironment(), this->getLua());
			this->onModHotReload(md->second.get());
			break;
		};
	}

	// LOAD -----
	void Scripting::loadLibraries() {
		if (this->_lua == nullptr) throw std::runtime_error("[RawrBox-Scripting] LUA is not set! Reference got destroyed?");

		this->_lua->open_libraries(sol::lib::base);
		this->_lua->open_libraries(sol::lib::package);
		this->_lua->open_libraries(sol::lib::math);
		this->_lua->open_libraries(sol::lib::table);
		this->_lua->open_libraries(sol::lib::debug);
		this->_lua->open_libraries(sol::lib::string);
		this->_lua->open_libraries(sol::lib::coroutine);
		this->_lua->open_libraries(sol::lib::bit32);

		this->_lua->open_libraries(sol::lib::jit);
	}

	void Scripting::loadTypes() {
		if (this->_lua == nullptr) throw std::runtime_error("[RawrBox-Scripting] LUA is not set! Reference got destroyed?");

		// Math ----
		rawrbox::AABBWrapper::registerLua(*this->_lua);
		rawrbox::ColorWrapper::registerLua(*this->_lua);
		rawrbox::MatrixWrapper::registerLua(*this->_lua);
		rawrbox::Vector2Wrapper::registerLua(*this->_lua);
		rawrbox::Vector3Wrapper::registerLua(*this->_lua);
		rawrbox::Vector4Wrapper::registerLua(*this->_lua);
		// ----

		// Default ----
		rawrbox::IOWrapper::registerLua(*this->_lua);
		rawrbox::FMTWrapper::registerLua(*this->_lua);
		rawrbox::ScriptingWrapper::registerLua(*this->_lua);
		rawrbox::ModWrapper::registerLua(*this->_lua);
		rawrbox::HooksWrapper::registerLua(*this->_lua);
		// ----

		// Custom ----
		this->onRegisterTypes();
		// ----
	}

	void Scripting::loadLuaExtensions(rawrbox::Mod* mod) {
		if (this->_lua == nullptr) throw std::runtime_error("[RawrBox-Scripting] LUA is not set! Reference got destroyed?");
		if (mod == nullptr) return;

		auto& env = mod->getEnvironment();

		this->loadLuaFile("./lua/table.lua", env, *this->_lua);
		this->loadLuaFile("./lua/string.lua", env, *this->_lua);
		this->loadLuaFile("./lua/math.lua", env, *this->_lua);
		this->loadLuaFile("./lua/json.lua", env, *this->_lua);
		this->loadLuaFile("./lua/sha2.lua", env, *this->_lua);
		this->loadLuaFile("./lua/util.lua", env, *this->_lua);
		this->loadLuaFile("./lua/input.lua", env, *this->_lua);

		// Custom ----
		this->onLoadExtensions(mod);
		// ----
	}

	bool Scripting::loadLuaFile(const std::string& path, const sol::environment& env, sol::state& lua) {
		if (!std::filesystem::exists(path)) {
			fmt::print("[RawrBox-Scripting] Failed to load lua : {}\n", path);
			return false;
		}

		std::string errStr;
		// try to load the file while handling exceptions
		auto ret = lua.safe_script_file(
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

	bool Scripting::isLuaFileMounted(const std::string& path) const {
		for (auto& pt : this->_loadedLuaFiles) {
			auto fnd = std::find(pt.second.begin(), pt.second.end(), path) != pt.second.end();
			if (fnd) return true;
		};

		return false;
	}
	//  -----

	// UTILS -----
	sol::state& Scripting::getLua() {
		return *this->_lua;
	}

	const std::unordered_map<std::string, std::unique_ptr<Mod>>& Scripting::getMods() const {
		return this->_mods;
	}

	const std::vector<std::string> Scripting::getModsIds() const {
		std::vector<std::string> modNames = {};
		for (auto& mod : this->_mods) {
			modNames.push_back(mod.second->getID());
		}

		return modNames;
	}

	bool Scripting::hotReloadEnabled() const {
		return this->_hotReloadEnabled;
	}
	// -----

} // namespace rawrbox
