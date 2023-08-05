#include <rawrbox/scripting/scripting.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/fmt_wrapper.hpp>
#include <rawrbox/scripting/wrappers/io_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/aabb_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/color_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/matrix_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector2_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector3_wrapper.hpp>
#include <rawrbox/scripting/wrappers/math/vector4_wrapper.hpp>

#include <filesystem>
#include <string>

namespace rawrbox {

	void Scripting::init() {
		this->_mods.clear();

		// Loading initial libs ---
		this->loadLibraries();
		this->loadTypes();
		// ----

		// Load mods ---
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
			if (!mod.second->load()) fmt::print("[RawrBox-Scripting] Failed to load mod '{}'\n", mod.first);
		}
		// -----

		this->call("init");
	}

	void Scripting::addGlobals(rawrbox::Mod& mod) {
		auto& env = mod.getEnvironment();

		env["print"] = [this](sol::variadic_args va) {
			auto vars = std::vector<sol::object>(va.begin(), va.end());

			std::vector<std::string> prtData;
			for (auto& var : vars) {
				prtData.push_back(this->_lua["tostring"](var));
			}

			if (prtData.empty()) return;
			fmt::print("{}\n", fmt::join(prtData, " "));
		};

		// Override require
		env["require"] = [&env](const std::string& path) {
			return env["include"](path);
		};

		// TODO: Handle relative paths (aka ./test/ui.lua inside of ./src/cl/init.lua, it should append this path, instead of requiring full path)
		env["include"] = [&env, &mod, this](const std::string& path) {
			auto old = this->_includePathPrefix;
			auto slashPos = path.find_last_of('/');
			if (slashPos != std::string::npos) {
				this->_includePathPrefix += path.substr(0, slashPos + 1);
			}

			auto merged = old + path;
			auto& modFolder = mod.getFolder();
			if (merged.substr(0, modFolder.size()) == modFolder) {
				merged = merged.substr(modFolder.size() + 1);
			}

			bool loaded = loadLuaFile(LuaUtils::getContent(merged, mod.getFolder()), env, this->getLua(), false);
			if (!loaded) fmt::print("[RawrBox-Scripting] Failed to load '{}'\n", path);

			this->_includePathPrefix = old;
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
		// -------------------

		// Global types ------------------------------------
		env["fmt"] = rawrbox::FMTWrapper();
		env["io"] = rawrbox::IOWrapper();
		// -------------------

		// Custom global types ---
		this->onRegisterGlobals(mod);
		// ----
	}

	void Scripting::update() {
		this->call("update");
	}

	void Scripting::fixedUpdate() {
		this->call("fixedUpdate");
	}

	// LOAD -----
	void Scripting::loadLibraries() {
		this->_lua.open_libraries(sol::lib::base);
		// lua.open_libraries(sol::lib::io); -> Use our own IO instead
		// lua.open_libraries(sol::lib::package); -> Use our own package instead
		this->_lua.open_libraries(sol::lib::math);
		this->_lua.open_libraries(sol::lib::table);
		this->_lua.open_libraries(sol::lib::debug);
		this->_lua.open_libraries(sol::lib::string);
		this->_lua.open_libraries(sol::lib::coroutine);
	}

	void Scripting::loadTypes() {
		// Default ----
		rawrbox::IOWrapper::registerLua(this->_lua);
		rawrbox::FMTWrapper::registerLua(this->_lua);
		// ----

		// Math ----
		rawrbox::AABBWrapper::registerLua(this->_lua);
		rawrbox::ColorWrapper::registerLua(this->_lua);
		rawrbox::MatrixWrapper::registerLua(this->_lua);
		rawrbox::Vector2Wrapper::registerLua(this->_lua);
		rawrbox::Vector3Wrapper::registerLua(this->_lua);
		rawrbox::Vector4Wrapper::registerLua(this->_lua);
		// ----

		// Custom ----
		this->onRegisterTypes();
		// ----
	}

	void Scripting::loadLuaExtensions(sol::environment& env, sol::state& lua) {
		this->loadLuaFile("./lua/extensions/table.lua", env, lua);
		this->loadLuaFile("./lua/extensions/string.lua", env, lua);
		this->loadLuaFile("./lua/extensions/math.lua", env, lua);
		this->loadLuaFile("./lua/extensions/json.lua", env, lua);
		this->loadLuaFile("./lua/extensions/sha2.lua", env, lua);
		this->loadLuaFile("./lua/extensions/util.lua", env, lua);
		this->loadLuaFile("./lua/extensions/input.lua", env, lua);
	}

	bool Scripting::loadLuaFile(const std::string& path, const sol::environment& env, sol::state& lua, bool setIncludeDirectory) {
		auto oldIncludeDir = this->_includePathPrefix;
		if (setIncludeDirectory) {
			auto slashPos = path.find_last_of('/');
			if (slashPos != std::string::npos) {
				this->_includePathPrefix = path.substr(0, slashPos + 1);
			}
		}

		if (!std::filesystem::exists(path)) {
			if (setIncludeDirectory) this->_includePathPrefix = oldIncludeDir;
			return false;
		}

		std::string errStr;

		// try to load the file while handling exceptions
		try {
			auto ret = lua.safe_script_file(path, env, sol::load_mode::text);
			if (!ret.valid()) {
				sol::error err = ret;
				errStr = err.what();
			} else {
				if (std::find(this->_loadedLuaFiles.begin(), this->_loadedLuaFiles.end(), path) == this->_loadedLuaFiles.end()) {
					this->_loadedLuaFiles.push_back(path); // Store for later comparison
				}
			}
		} catch (const std::exception& err) {
			errStr = err.what();
		}

		if (setIncludeDirectory) this->_includePathPrefix = oldIncludeDir;

		// check if we loaded the file
		if (errStr.empty()) return true;

		fmt::print("[RawrBox-Scripting] Lua error: {}\n", errStr);
		return false;
	}

	bool Scripting::isLuaFileMounted(const std::string& path) const {
		return std::find(this->_loadedLuaFiles.begin(), this->_loadedLuaFiles.end(), path) != this->_loadedLuaFiles.end();
	}
	//  -----

	// UTILS -----
	sol::state& Scripting::getLua() {
		return this->_lua;
	}

	const std::unordered_map<std::string, std::unique_ptr<Mod>>& Scripting::getMods() const {
		return this->_mods;
	}

	const std::vector<std::string> Scripting::getModsIds() const {
		std::vector<std::string> modNames = {};
		for (auto& mod : this->_mods)
			modNames.push_back(mod.second->getID());

		return modNames;
	}
	// -----

	void Scripting::setIncludePath(const std::string& path) {
		this->_includePathPrefix = path;
	}

	const std::string& Scripting::getIncludePath() const {
		return this->_includePathPrefix;
	}

} // namespace rawrbox
