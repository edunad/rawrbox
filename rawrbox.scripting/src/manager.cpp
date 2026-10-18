#include <rawrbox/engine/static.hpp>
#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/console.hpp>
#include <rawrbox/scripting/wrappers/console_command.hpp>
#include <rawrbox/scripting/wrappers/hooks.hpp>
#include <rawrbox/scripting/wrappers/i18n.hpp>
#include <rawrbox/scripting/wrappers/io.hpp>
#include <rawrbox/scripting/wrappers/math/aabb.hpp>
#include <rawrbox/scripting/wrappers/math/bbox.hpp>
#include <rawrbox/scripting/wrappers/math/color.hpp>
#include <rawrbox/scripting/wrappers/math/matrix.hpp>
#include <rawrbox/scripting/wrappers/math/vector2.hpp>
#include <rawrbox/scripting/wrappers/math/vector3.hpp>
#include <rawrbox/scripting/wrappers/math/vector4.hpp>
#include <rawrbox/scripting/wrappers/mod.hpp>
#include <rawrbox/scripting/wrappers/timer.hpp>
#include <rawrbox/utils/i18n.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/time.hpp>

#include <fmt/args.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <vector>

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
	std::unordered_map<std::string, std::vector<std::filesystem::path>> SCRIPTING::_loadedLuaFiles = {};

	std::unique_ptr<rawrbox::FileWatcher> SCRIPTING::_watcher = nullptr;
	std::unique_ptr<rawrbox::LuaChecker> SCRIPTING::_checker = nullptr;
	std::vector<std::unique_ptr<rawrbox::ScriptingPlugin>> SCRIPTING::_plugins = {};

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> SCRIPTING::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SCRIPTING");
	// -------------

	rawrbox::Console* SCRIPTING::_console = nullptr;
	bool SCRIPTING::_hotReloadEnabled = false;
	// --------------

	// PUBLIC ----
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onRegisterTypes;
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onRegisterGlobals;
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onLoadLibraries;
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onLoadModifiers;
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onModHotReload;
	rawrbox::Event<rawrbox::Mod&> SCRIPTING::onModUnload;

	bool SCRIPTING::initialized = false;
	// ------

	// LOAD -----
	void SCRIPTING::loadLibraries(rawrbox::Mod& mod) {
		auto* L = mod.getEnvironment();
		if (L == nullptr) RAWRBOX_CRITICAL("LUA is not set! Reference got destroyed?");

		// COMMON -----
		luaL_openlibs(L); // Should be safe, since LUAU takes care of non-secure libs (https://luau-lang.org/sandbox#library)
				  //  -----

		// OTHER LIBS ---
		rawrbox::LuaUtils::compileAndLoadFile(L, "SHA", "./lua/sha2.lua");
		rawrbox::LuaUtils::compileAndLoadFile(L, "JSON", "./lua/json.lua");
		// --------------

		// Rawrbox LIBS ----
		rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Math", "./lua/math.lua");
		rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::String", "./lua/string.lua");
		rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Table", "./lua/table.lua");
		// -----------------

		// Rawrbox enums ---
		if (_console != nullptr) rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::Console", "./lua/enums/console.lua");
		rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::Input", "./lua/enums/input.lua");
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
		auto* L = mod.getEnvironment();
		if (L == nullptr) RAWRBOX_CRITICAL("LUA is not set! Reference got destroyed?");
		// Register types, these will be read-only & sandboxed!

		// Rawrbox ---
		rawrbox::Hooks::registerLua(L);
		rawrbox::TimerWrapper::registerLua(L);
		rawrbox::I18NWrapper::registerLua(L);
		rawrbox::MODWrapper::registerLua(L);

#ifdef RAWRBOX_SCRIPTING_UNSAFE
		rawrbox::IOWrapper::registerLua(L); // TODO: Might have security vulnerabilities
#endif

		if (_console != nullptr) {
			rawrbox::ConsoleCommandWrapper::registerLua(L);
			rawrbox::ConsoleWrapper::registerLua(L);
		}
		// -----------

		// Rawrbox Math
		rawrbox::Vector2Wrapper::registerLua(L);
		rawrbox::Vector3Wrapper::registerLua(L);
		rawrbox::Vector4Wrapper::registerLua(L);

		rawrbox::ColorWrapper::registerLua(L);
		rawrbox::AABBWrapper::registerLua(L);
		rawrbox::BBOXWrapper::registerLua(L);

		rawrbox::MatrixWrapper::registerLua(L);
		// -------------

		// Register plugins types ---
		for (auto& p : _plugins)
			p->registerTypes(L);
		//  -----

		// Custom ----
		onRegisterTypes(mod);
		// ----
	}

	void SCRIPTING::loadGlobals(rawrbox::Mod& mod) {
		auto* L = mod.getEnvironment();
		if (L == nullptr) RAWRBOX_CRITICAL("LUA is not set! Reference got destroyed?");
		// Register globals, these will be read-only & sandboxed!

		// OVERRIDES ----
		luabridge::getGlobalNamespace(L)
		    .addFunction("printTable", [](const luabridge::LuaRef& ref) {
			    std::string output;

			    glz::error_ctx ec = glz::write<glz::opts{.prettify = 1U}>(rawrbox::LuaUtils::luaToJsonObject(ref), output);
			    if (ec) throw std::runtime_error(fmt::format("Failed to printTable"));

			    _logger->info("{}", output);
		    })
		    .addFunction("print", [](lua_State* state) {
			    auto args = rawrbox::LuaUtils::argsToString(state);
			    if (args.empty()) return;

			    _logger->info("{}", fmt::join(args, " "));
		    })
		    .addFunction("include", [&mod](lua_State* state) {
			    if (lua_type(state, 1) != LUA_TSTRING) throw std::runtime_error("Invalid param, string expected");
			    const auto* path = lua_tostring(state, 1);

			    auto fixedPath = LuaUtils::getContent(path, mod.getFolder().generic_string());
			    if (!fixedPath.first.empty()) throw std::runtime_error("External mod lua loading not supported");
			    loadFile(mod, state, fixedPath.second);

			    // Register file for hot-reloading
			    registerLoadedFile(mod.getID(), fixedPath.second);
			    // ----
		    });
		// ---------

		// TIME UTILS ---
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("time", {})
		    .addFunction("curtime", []() { return rawrbox::TimeUtils::curtime(); })
		    .addFunction("time", []() { return rawrbox::TimeUtils::time(); })
		    .endNamespace();
		// -------------

		// MODDING ---
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("mod", {})
		    .addFunction("exists", [](const std::string& id) {
			    return _mods.find(id) != _mods.end();
		    })
		    .addFunction("get", [](const std::string& id) {
			    auto fnd = _mods.find(id);
			    if (fnd == _mods.end()) throw std::runtime_error(fmt::format("Mod {} not found", id));

			    return rawrbox::MODWrapper(fnd->second.get());
		    })
		    .endNamespace();
		// ----------

		// STRING ----
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("string", {})
		    .addFunction("vformat", [](lua_State* state) {
			    auto vars = rawrbox::LuaUtils::argsToString(state);
			    if (vars.empty()) throw std::runtime_error("Missing params");

			    fmt::dynamic_format_arg_store<fmt::format_context> fmtArgs = {};
			    for (size_t i = 1; i < vars.size(); i++) {
				    fmtArgs.push_back(vars[i]);
			    }

			    return fmt::vformat(vars[0], fmtArgs);
		    })
		    .endNamespace();
		// -----------

		// ENGINE ------FRAME_ALPHA
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("engine", {})
		    .addProperty("deltaTime", &rawrbox::DELTA_TIME)
		    .addProperty("fixedDeltaTime", &rawrbox::FIXED_DELTA_TIME)
		    .addProperty("frameAlpha", &rawrbox::FRAME_ALPHA)
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

	void SCRIPTING::loadModifiers(rawrbox::Mod& mod) {
		for (auto& p : _plugins) {
			p->modifyMod(mod);
		}

		// Custom ----
		onLoadModifiers(mod);
		// -------------
	}
	// -------------

	// MOD LOAD ---
	void SCRIPTING::loadI18N(const rawrbox::Mod& mod) {
		auto i18nPath = mod.getFolder() / "i18n";
		if (!std::filesystem::exists(i18nPath)) return;

		rawrbox::I18N::loadLanguagePack(mod.getID(), i18nPath);
	}
	// ------------

	// HOT RELOAD -----
	void SCRIPTING::registerLoadedFile(const std::string& modId, const std::filesystem::path& filePath) {
		auto mdFnd = _loadedLuaFiles.find(modId);

		if (mdFnd != _loadedLuaFiles.end()) {
			auto fileFnd = std::find(mdFnd->second.begin(), mdFnd->second.end(), filePath);
			if (fileFnd != mdFnd->second.end()) return; // Already registered

			mdFnd->second.push_back(filePath);
		} else {
			_loadedLuaFiles[modId] = {filePath};
		}

		if (hotReloadEnabled()) {
			_logger->debug("Registered {} -> {} for hot reload", fmt::styled(modId, fmt::fg(fmt::color::coral)), filePath.generic_string());
			_watcher->watchFile(filePath);
		}
	}

	void SCRIPTING::loadFile(rawrbox::Mod& mod, lua_State* L, const std::filesystem::path& filePath) {
		for (auto& p : _plugins) {
			if (p->loadFile(mod, L, filePath)) return;
		}

		rawrbox::LuaUtils::compileAndLoadFile(L, mod.getID(), filePath);
	}

	void SCRIPTING::hotReload(const std::filesystem::path& filePath) {
		// Find the owner
		for (auto& pt : _loadedLuaFiles) {
			const auto fnd = std::ranges::find(pt.second, filePath) != pt.second.end();
			if (!fnd) continue;

			const auto md = _mods.find(pt.first);
			if (md == _mods.end()) return;

			_logger->warn("Hot-reloading lua file '{}'", filePath.generic_string());
			if (_checker != nullptr && !_checker->check(filePath).empty()) {
				_logger->error("Hot-reload of '{}' skipped, fix the type errors above", filePath.generic_string());
				return;
			}

			// Cleanup and load -----
			md->second->gc(); // Cleanup

			try {
				loadFile(*md->second, md->second->getEnvironment(), filePath);
			} catch (const std::exception& err) {
				_logger->error("{}", err.what());
			}
			// ---------------

			onModHotReload(*md->second);
			break;
		};
	}
	// -------------
	// ----------

	void SCRIPTING::init(const rawrbox::ScriptingSettings& settings) {
		_hotReloadEnabled = settings.hotReloadMs > 0;
		if (_hotReloadEnabled) {
			_logger->info("Enabled lua hot-reloading\n  └── Delay: {}ms", fmt::styled(settings.hotReloadMs, fmt::fg(fmt::color::gold)));

			_watcher = std::make_unique<rawrbox::FileWatcher>(
			    [](const std::string& pth, rawrbox::FileStatus status) {
				    if (status != rawrbox::FileStatus::modified) return;
				    rawrbox::runOnRenderThread([pth]() { hotReload(pth); });
			    },
			    std::chrono::milliseconds(settings.hotReloadMs));
			_watcher->start();
		}

		if (settings.typeCheck) {
			try {
				_checker = std::make_unique<rawrbox::LuaChecker>(settings.definitions);
			} catch (const std::runtime_error& err) {
				RAWRBOX_CRITICAL("{}", err.what());
			}

			_logger->info("Enabled lua type checking\n  └── Definitions: {}", fmt::styled(settings.definitions.size(), fmt::fg(fmt::color::gold)));
		}

		// Setup  --
		if (_console != nullptr) rawrbox::ConsoleWrapper::init(_console);
		// ----------------
	}

	// LOADING ----
	std::unordered_map<std::filesystem::path, rawrbox::Mod*> SCRIPTING::loadMods(const std::filesystem::path& rootFolder) { // Load mods
		if (!std::filesystem::exists(rootFolder)) RAWRBOX_CRITICAL("Failed to locate root folder '{}'", rootFolder.generic_string());

		// LOAD ORDER (higher priority first) ---
		std::vector<std::pair<short, std::filesystem::path>> folders = {};
		for (const auto& p : std::filesystem::directory_iterator(rootFolder)) {
			if (!p.is_directory()) continue;
			folders.emplace_back(loadMetadata(p.path()).priority, p.path());
		}

		std::ranges::stable_sort(folders, std::ranges::greater{}, &std::pair<short, std::filesystem::path>::first);
		// ------------

		std::unordered_map<std::filesystem::path, rawrbox::Mod*> success = {};
		for (const auto& folder : folders) {
			auto* mod = loadMod(folder.second.filename().generic_string(), folder.second);
			if (mod != nullptr) success[folder.second] = mod;
		}

		return success;
	}

	rawrbox::Mod* SCRIPTING::loadMod(const std::string& id, const std::filesystem::path& modFolder) {
		if (id.empty()) RAWRBOX_CRITICAL("Mod ID cannot be empty");
		if (!std::filesystem::exists(modFolder)) RAWRBOX_CRITICAL("Failed to locate mod folder '{}'", modFolder.generic_string());

		if (_mods.contains(id)) {
			_logger->warn("Mod {} already loaded! Mod name conflict?", id);
			return nullptr;
		}

		if (_checker != nullptr && !_checker->checkFolder(modFolder).empty()) {
			_logger->error("Mod '{}' failed to load, fix the type errors above", id);
			return nullptr;
		}

		auto mod = std::make_unique<rawrbox::Mod>(id, modFolder, loadMetadata(modFolder));

		// Prepare env ----------
		loadLibraries(*mod);
		loadTypes(*mod);
		loadGlobals(*mod);
		loadI18N(*mod);
		loadModifiers(*mod);
		// ----------

		try {
			mod->init(); // Sandbox env
			mod->load();

			mod->call("onInit");

			_logger->info("Mod '{}' loaded", fmt::styled(id, fmt::fg(fmt::color::coral)));
			registerLoadedFile(mod->getID(), mod->getEntryFilePath()); // Register file for hot-reloading
		} catch (const std::runtime_error& err) {
			_logger->error("{}", err.what());
			return nullptr;
		}

		rawrbox::Mod* modPtr = mod.get();
		_mods.emplace(id, std::move(mod));

		return modPtr;
	}

	rawrbox::ModMetadata SCRIPTING::loadMetadata(const std::filesystem::path& modFolder) {
		const auto configPath = fmt::format("{}/mod.json", modFolder.generic_string());

		rawrbox::ModMetadata metadata = {};
		if (!std::filesystem::exists(configPath)) return metadata;

		std::string buffer = {};
		const auto meta = glz::read_file_json<glz::opts{.comments = 1U, .error_on_unknown_keys = 0U, .skip_null_members = 1U, .error_on_missing_keys = 0U}>(metadata, configPath, buffer);
		if (meta) RAWRBOX_CRITICAL("Failed to load '{}' ──> {}", configPath, glz::format_error(meta, buffer));

		return metadata;
	}

	bool SCRIPTING::unloadMod(const std::filesystem::path& modFolder) {
		return unloadMod(modFolder.filename().generic_string());
	}

	bool SCRIPTING::unloadMod(const std::string& modId) {
		const auto fnd = _mods.find(modId);
		if (fnd == _mods.end()) return false;

		const auto fndLua = _loadedLuaFiles.find(modId);
		if (fndLua != _loadedLuaFiles.end()) {
			if (_watcher != nullptr) {
				for (auto& pt : fndLua->second) {
					_watcher->unwatchFile(pt);
				}
			}

			_loadedLuaFiles.erase(fndLua);
		}

		onModUnload(*fnd->second);

		fnd->second->shutdown();
		_mods.erase(fnd);

		_logger->info("Mod '{}' unloaded", fmt::styled(modId, fmt::fg(fmt::color::coral)));
		return true;
	}
	// -----------

	void SCRIPTING::shutdown() {
		// Shutdown mods ---
		for (const auto& mod : _mods) {
			mod.second->shutdown();
		}
		// ----------------

		_console = nullptr;
		_watcher.reset();
		_checker.reset();

		_loadedLuaFiles.clear();
		_mods.clear();
		_plugins.clear();
	}

	void SCRIPTING::setConsole(rawrbox::Console* console) { _console = console; }

	// UTILS ----
	bool SCRIPTING::hasMod(const std::string& id) {
		return _mods.contains(id);
	}

	rawrbox::Mod* SCRIPTING::getMod(const std::string& id) {
		const auto fnd = _mods.find(id);
		if (fnd == _mods.end()) return nullptr;

		return fnd->second.get();
	}

	const std::unordered_map<std::string, std::unique_ptr<Mod>>& SCRIPTING::getMods() { return _mods; }
	std::vector<std::string> SCRIPTING::getModsIds() {
		std::vector<std::string> modNames = {};
		modNames.reserve(_mods.size());
		for (const auto& mod : _mods) {
			modNames.push_back(mod.second->getID());
		}

		return modNames;
	}

	bool SCRIPTING::hotReloadEnabled() { return _hotReloadEnabled; }
	bool SCRIPTING::isLuaFileMounted(const std::string& path) {
		for (auto& pt : _loadedLuaFiles) {
			const auto fnd = std::ranges::find(pt.second, path) != pt.second.end();
			if (fnd) return true;
		};

		return false;
	}

#ifdef RAWRBOX_SCRIPTING_WORKSHOP_MODDING
	std::vector<rawrbox::Mod*> SCRIPTING::getModsByWorkshopType(uint32_t type) {
		std::vector<rawrbox::Mod*> mods = {};
		for (auto& mod : _mods) {
			if (mod.second->getWorkshopType() == type) mods.push_back(mod.second.get());
		}

		return mods;
	}

	rawrbox::Mod* SCRIPTING::getModByWorkshopId(uint64_t id) {
		for (auto& mod : _mods) {
			if (mod.second->getWorkshopId() == id) return mod.second.get();
		}

		return nullptr;
	}
#endif
	// -----
} // namespace rawrbox