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
#include <rawrbox/scripting/wrappers/profiler.hpp>
#include <rawrbox/scripting/wrappers/timer.hpp>
#include <rawrbox/utils/i18n.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/time.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <fmt/args.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <ranges>
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
	std::vector<std::unique_ptr<rawrbox::Mod>> SCRIPTING::_mods = {};
	std::unordered_map<std::string, std::vector<std::filesystem::path>> SCRIPTING::_loadedLuaFiles = {};

	std::unique_ptr<rawrbox::FileWatcher> SCRIPTING::_watcher = nullptr;
	std::unique_ptr<rawrbox::LuaChecker> SCRIPTING::_checker = nullptr;
	std::vector<std::unique_ptr<rawrbox::ScriptingPlugin>> SCRIPTING::_plugins = {};

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> SCRIPTING::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SCRIPTING");
	// -------------

	rawrbox::Console* SCRIPTING::_console = nullptr;
	bool SCRIPTING::_hotReloadEnabled = false;
	bool SCRIPTING::_safeEnv = true;
	uint32_t SCRIPTING::_libraries = rawrbox::ScriptingLibraries::ALL;
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
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::SHA2)) rawrbox::LuaUtils::compileAndLoadFile(L, "SHA", "./lua/sha2.lua");
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::JSON)) rawrbox::LuaUtils::compileAndLoadFile(L, "JSON", "./lua/json.lua");
		// --------------

		// Rawrbox LIBS ----
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::MATH)) rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Math", "./lua/math.lua");
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::STRING)) rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::String", "./lua/string.lua");
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::TABLE)) rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Table", "./lua/table.lua");
		// -----------------

		// Rawrbox enums ---
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::ENUMS)) {
			if (SCRIPTING::_console != nullptr) rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::Console", "./lua/enums/console.lua");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::Input", "./lua/enums/input.lua");
		}
		// -----------------

		// Register plugins libraries ---
		for (auto& p : SCRIPTING::_plugins)
			p->loadLibraries(L);
		//  -----

		// Custom ----
		SCRIPTING::onLoadLibraries(mod);
		// ----
	}

	void SCRIPTING::loadTypes(rawrbox::Mod& mod) {
		auto* L = mod.getEnvironment();
		if (L == nullptr) RAWRBOX_CRITICAL("LUA is not set! Reference got destroyed?");
		// Register types, these will be read-only & sandboxed!

		// Rawrbox ---
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::HOOKS)) rawrbox::Hooks::registerLua(L);
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::TIMER)) rawrbox::TimerWrapper::registerLua(L);
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::PROFILER)) rawrbox::ProfilerWrapper::registerLua(L);
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::I18N)) rawrbox::I18NWrapper::registerLua(L);
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::MOD)) rawrbox::MODWrapper::registerLua(L);

#ifdef RAWRBOX_SCRIPTING_UNSAFE
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::IO)) rawrbox::IOWrapper::registerLua(L); // TODO: Might have security vulnerabilities
#endif

		if (SCRIPTING::_console != nullptr && SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::CONSOLE)) {
			rawrbox::ConsoleCommandWrapper::registerLua(L);
			rawrbox::ConsoleWrapper::registerLua(L);
		}
		// -----------

		// Rawrbox Math
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::MATH_TYPES)) {
			rawrbox::Vector2Wrapper::registerLua(L);
			rawrbox::Vector3Wrapper::registerLua(L);
			rawrbox::Vector4Wrapper::registerLua(L);

			rawrbox::ColorWrapper::registerLua(L);
			rawrbox::AABBWrapper::registerLua(L);
			rawrbox::BBOXWrapper::registerLua(L);

			rawrbox::MatrixWrapper::registerLua(L);
		}
		// -------------

		// Register plugins types ---
		for (auto& p : SCRIPTING::_plugins)
			p->registerTypes(L);
		//  -----

		// Custom ----
		SCRIPTING::onRegisterTypes(mod);
		// ----
	}

	void SCRIPTING::loadGlobals(rawrbox::Mod& mod) {
		auto* L = mod.getEnvironment();
		if (L == nullptr) RAWRBOX_CRITICAL("LUA is not set! Reference got destroyed?");
		// Register globals, these will be read-only & sandboxed!

		// OVERRIDES ----
		luabridge::getGlobalNamespace(L)
		    .addFunction("printTable", [](const luabridge::LuaRef& ref) {
			    const auto json = rawrbox::LuaUtils::toJSON(ref, {.strict = false});
			    if (!json) throw std::runtime_error(fmt::format("Failed to printTable ──> {}", json.error()));

			    std::string output;
			    glz::error_ctx ec = glz::write<glz::opts{.prettify = 1U}>(json.value(), output);
			    if (ec) throw std::runtime_error(fmt::format("Failed to printTable"));

			    SCRIPTING::_logger->info("{}", output);
		    })
		    .addFunction("print", [](lua_State* state) {
			    auto args = rawrbox::LuaUtils::argsToString(state);
			    if (args.empty()) return;

			    SCRIPTING::_logger->info("{}", fmt::join(args, " "));
		    })
		    .addFunction("include", [&mod](lua_State* state) {
			    if (lua_type(state, 1) != LUA_TSTRING) throw std::runtime_error("Invalid param, string expected");
			    const auto* path = lua_tostring(state, 1);

			    const auto fixedPath = LuaUtils::resolveContent(path, mod);
			    if (!fixedPath) throw std::runtime_error(fmt::format("Failed to include '{}' ──> {}", path, fixedPath.error()));
			    SCRIPTING::loadFile(mod, state, fixedPath.value());

			    // Register file for hot-reloading
			    SCRIPTING::registerLoadedFile(mod.getID(), fixedPath.value());
			    // ----
		    });
		// ---------

		// TIME UTILS ---
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::TIME)) {
			luabridge::getGlobalNamespace(L)
			    .beginNamespace("time", {})
			    .addFunction("curtime", []() { return rawrbox::TimeUtils::curtime(); })
			    .addFunction("time", []() { return rawrbox::TimeUtils::time(); })
			    .endNamespace();
		}
		// -------------

		// MODDING ---
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::MOD)) {
			luabridge::getGlobalNamespace(L)
			    .beginNamespace("mod", {})
			    .addFunction("exists", [](const std::string& id) {
				    return SCRIPTING::hasMod(id);
			    })
			    .addFunction("get", [](const std::string& id) {
				    auto* modWrap = SCRIPTING::getMod(id);
				    if (modWrap == nullptr) throw std::runtime_error(fmt::format("Mod {} not found", id));

				    return rawrbox::MODWrapper(modWrap);
			    })
			    .endNamespace();
		}
		// ----------

		// STRING ----
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::STRING)) {
			luabridge::getGlobalNamespace(L)
			    .beginNamespace("string", {})
			    .addFunction("vformat", [](lua_State* state) {
				    const auto vars = rawrbox::LuaUtils::argsToString(state);
				    if (vars.empty()) throw std::runtime_error("Missing params");

				    fmt::dynamic_format_arg_store<fmt::format_context> fmtArgs = {};
				    for (size_t i = 1; i < vars.size(); i++) {
					    fmtArgs.push_back(vars[i]);
				    }

				    return fmt::vformat(vars[0], fmtArgs);
			    })
			    .endNamespace();
		}
		// -----------

		// ENGINE ------FRAME_ALPHA
		if (SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::ENGINE)) {
			luabridge::getGlobalNamespace(L)
			    .beginNamespace("engine", {})
			    .addProperty("deltaTime", &rawrbox::DELTA_TIME)
			    .addProperty("fixedDeltaTime", &rawrbox::FIXED_DELTA_TIME)
			    .addProperty("frameAlpha", &rawrbox::FRAME_ALPHA)
			    .endNamespace();
		}
		// -------------

		// Register plugins globals ---
		for (auto& p : SCRIPTING::_plugins)
			p->registerGlobal(L);
		//  -----

		// Custom ----
		SCRIPTING::onRegisterGlobals(mod);
		// -------------
	}

	void SCRIPTING::loadModifiers(rawrbox::Mod& mod) {
		for (auto& p : SCRIPTING::_plugins) {
			p->modifyMod(mod);
		}

		// Custom ----
		SCRIPTING::onLoadModifiers(mod);
		// -------------
	}
	// -------------

	// MOD LOAD ---
	void SCRIPTING::loadI18N(const rawrbox::Mod& mod) {
		const auto i18nPath = fmt::format("{}/i18n", mod.getFolder().generic_string());
		if (!rawrbox::VFS::contains(i18nPath)) return;

		rawrbox::I18N::loadLanguagePack(mod.getID(), i18nPath);
	}
	// ------------

	// HOT RELOAD -----
	void SCRIPTING::registerLoadedFile(const std::string& modId, const std::filesystem::path& filePath) {
		const auto& mdFnd = SCRIPTING::_loadedLuaFiles.find(modId);

		if (mdFnd != SCRIPTING::_loadedLuaFiles.end()) {
			const auto& fileFnd = std::ranges::find(mdFnd->second, filePath);
			if (fileFnd != mdFnd->second.end()) return; // Already registered

			mdFnd->second.push_back(filePath);
		} else {
			SCRIPTING::_loadedLuaFiles[modId] = {filePath};
		}

		if (SCRIPTING::hotReloadEnabled()) {
			SCRIPTING::_logger->debug("Registered {} -> {} for hot reload", fmt::styled(modId, fmt::fg(fmt::color::coral)), filePath.generic_string());
			SCRIPTING::_watcher->watchFile(filePath);
		}
	}

	void SCRIPTING::loadFile(rawrbox::Mod& mod, lua_State* L, const std::filesystem::path& filePath) {
		for (auto& p : SCRIPTING::_plugins) {
			if (p->loadFile(mod, L, filePath)) return;
		}

		rawrbox::LuaUtils::compileAndLoadFile(L, mod.getID(), filePath);
	}

	void SCRIPTING::hotReload(const std::filesystem::path& filePath) {
		// Find the owner
		for (auto& pt : SCRIPTING::_loadedLuaFiles) {
			const auto& fnd = std::ranges::find(pt.second, filePath) != pt.second.end();
			if (!fnd) continue;

			auto* mod = SCRIPTING::getMod(pt.first);
			if (mod == nullptr) return;

			SCRIPTING::_logger->warn("Hot-reloading lua file '{}'", filePath.generic_string());
			if (SCRIPTING::_checker != nullptr && !SCRIPTING::_checker->check(filePath).empty()) {
				SCRIPTING::_logger->error("Hot-reload of '{}' skipped, fix the type errors above", filePath.generic_string());
				return;
			}

			// Cleanup and load -----
			mod->gc(); // Cleanup

			try {
				SCRIPTING::loadFile(*mod, mod->getEnvironment(), filePath);
			} catch (const std::exception& err) {
				SCRIPTING::_logger->error("{}", err.what());
			}
			// ---------------

			SCRIPTING::onModHotReload(*mod);
			break;
		};
	}
	// -------------
	// ----------

	void SCRIPTING::init(const rawrbox::ScriptingSettings& settings) {
		SCRIPTING::_safeEnv = settings.safeEnv;
		SCRIPTING::_libraries = settings.libraries;
		SCRIPTING::_hotReloadEnabled = settings.hotReloadMs > 0;

		if (SCRIPTING::_hotReloadEnabled) {
			SCRIPTING::_logger->info("Enabled lua hot-reloading\n  └── Delay: {}ms", fmt::styled(settings.hotReloadMs, fmt::fg(fmt::color::gold)));

			SCRIPTING::_watcher = std::make_unique<rawrbox::FileWatcher>(
			    [](const std::string& pth, rawrbox::FileStatus status) {
				    if (status != rawrbox::FileStatus::modified) return;
				    rawrbox::runOnRenderThread([pth]() { SCRIPTING::hotReload(pth); });
			    },
			    std::chrono::milliseconds(settings.hotReloadMs));
			SCRIPTING::_watcher->start();
		}

		if (settings.typeCheck) {
			try {
				SCRIPTING::_checker = std::make_unique<rawrbox::LuaChecker>(settings.definitions);
			} catch (const std::runtime_error& err) {
				RAWRBOX_CRITICAL("{}", err.what());
			}

			SCRIPTING::_logger->info("Enabled lua type checking\n  └── Definitions: {}", fmt::styled(settings.definitions.size(), fmt::fg(fmt::color::gold)));
		}

		// Setup  --
		if (SCRIPTING::_console != nullptr) rawrbox::ConsoleWrapper::init(SCRIPTING::_console);
		// ----------------
	}

	// LOADING ----
	std::vector<rawrbox::Mod*> SCRIPTING::loadMods(const std::filesystem::path& rootFolder) { // Load mods
		if (!std::filesystem::exists(rootFolder)) RAWRBOX_CRITICAL("Failed to locate root folder '{}'", rootFolder.generic_string());

		// LOAD ORDER = rank order ---
		std::vector<std::pair<short, std::filesystem::path>> folders = {};
		for (const auto& p : std::filesystem::directory_iterator(rootFolder)) {
			if (!p.is_directory()) continue;

			const auto metadata = SCRIPTING::loadMetadata(p.path());
			folders.emplace_back(metadata ? metadata->priority : 0, p.path()); // Broken metadata is reported by loadMod
		}

		std::ranges::sort(folders, [](const auto& a, const auto& b) {
			if (a.first != b.first) return a.first > b.first;
			return a.second.filename() < b.second.filename();
		});
		// ------------

		std::vector<rawrbox::Mod*> loaded = {};
		for (const auto& folder : folders) {
			auto* mod = SCRIPTING::loadMod(folder.second.filename().generic_string(), folder.second);
			if (mod != nullptr) loaded.push_back(mod);
		}

		return loaded;
	}

	rawrbox::Mod* SCRIPTING::loadMod(const std::string& id, const std::filesystem::path& modFolder) {
		if (id.empty()) RAWRBOX_CRITICAL("Mod ID cannot be empty");
		if (!std::filesystem::exists(modFolder)) RAWRBOX_CRITICAL("Failed to locate mod folder '{}'", modFolder.generic_string());

		if (SCRIPTING::hasMod(id)) {
			SCRIPTING::_logger->warn("Mod {} already loaded! Mod name conflict?", id);
			return nullptr;
		}

		auto metadata = SCRIPTING::loadMetadata(modFolder);
		if (!metadata) {
			SCRIPTING::_logger->error("Mod '{}' failed to load\n  └── {}", id, metadata.error());
			return nullptr;
		}

		if (SCRIPTING::_checker != nullptr && !SCRIPTING::_checker->checkFolder(modFolder).empty()) {
			SCRIPTING::_logger->error("Mod '{}' failed to load, fix the type errors above", id);
			return nullptr;
		}

		auto mod = std::make_unique<rawrbox::Mod>(id, modFolder, std::move(metadata.value()));

		// Prepare env ----------
		SCRIPTING::loadLibraries(*mod);
		SCRIPTING::loadTypes(*mod);
		SCRIPTING::loadGlobals(*mod);
		SCRIPTING::loadI18N(*mod);
		SCRIPTING::loadModifiers(*mod);
		// ----------

		try {
			mod->init(); // Sandbox env
			if (!SCRIPTING::_safeEnv) lua_setsafeenv(mod->getEnvironment(), LUA_GLOBALSINDEX, 0);

			mod->load();
			mod->call("onInit");

			SCRIPTING::_logger->info("Mod '{}' loaded", fmt::styled(id, fmt::fg(fmt::color::coral)));
			SCRIPTING::registerLoadedFile(mod->getID(), mod->getEntryFilePath()); // Register file for hot-reloading
		} catch (const std::exception& err) {
			SCRIPTING::_logger->error("{}", err.what());
			return nullptr;
		}

		auto* modPtr = mod.get();
		const auto rank = std::ranges::upper_bound(SCRIPTING::_mods, *modPtr, SCRIPTING::rankBefore, [](const auto& loaded) -> const rawrbox::Mod& { return *loaded; });
		SCRIPTING::_mods.insert(rank, std::move(mod));

		return modPtr;
	}

	std::expected<rawrbox::ModMetadata, std::string> SCRIPTING::loadMetadata(const std::filesystem::path& modFolder) {
		const auto configPath = fmt::format("{}/mod.json", modFolder.generic_string());

		rawrbox::ModMetadata metadata = {};
		if (!std::filesystem::exists(configPath)) return metadata;

		std::string buffer = {};
		const auto meta = glz::read_file_json<glz::opts{.comments = 1U, .error_on_unknown_keys = 0U, .skip_null_members = 1U, .error_on_missing_keys = 0U}>(metadata, configPath, buffer);
		if (meta) return std::unexpected(fmt::format("Failed to load '{}' ──> {}", configPath, glz::format_error(meta, buffer)));

		return metadata;
	}

	bool SCRIPTING::rankBefore(const rawrbox::Mod& a, const rawrbox::Mod& b) {
		if (a.getMetadata().priority != b.getMetadata().priority) return a.getMetadata().priority > b.getMetadata().priority;
		return a.getID() < b.getID();
	}

	bool SCRIPTING::unloadMod(const std::filesystem::path& modFolder) {
		return SCRIPTING::unloadMod(modFolder.filename().generic_string());
	}

	bool SCRIPTING::unloadMod(const std::string& modId) {
		const auto& fnd = std::ranges::find(SCRIPTING::_mods, modId, [](const auto& mod) -> const std::string& {
			return mod->getID();
		});

		if (fnd == SCRIPTING::_mods.end()) return false;

		const auto& fndLua = SCRIPTING::_loadedLuaFiles.find(modId);
		if (fndLua != SCRIPTING::_loadedLuaFiles.end()) {
			if (SCRIPTING::_watcher != nullptr) {
				for (auto& pt : fndLua->second) {
					SCRIPTING::_watcher->unwatchFile(pt);
				}
			}

			SCRIPTING::_loadedLuaFiles.erase(fndLua);
		}

		auto& mod = **fnd;
		SCRIPTING::onModUnload(mod);
		rawrbox::Hooks::clear(mod.getEnvironment());

		mod.shutdown();
		SCRIPTING::_mods.erase(fnd);

		SCRIPTING::_logger->info("Mod '{}' unloaded", fmt::styled(modId, fmt::fg(fmt::color::coral)));
		return true;
	}
	// -----------

	void SCRIPTING::shutdown() {
		rawrbox::Hooks::clear();

		// Shutdown mods, reverse order ---
		for (auto& mod : std::views::reverse(SCRIPTING::_mods)) {
			SCRIPTING::onModUnload(*mod);
			mod->shutdown();
		}
		// ----------------

		SCRIPTING::_console = nullptr;
		SCRIPTING::_watcher.reset();
		SCRIPTING::_checker.reset();

		SCRIPTING::_loadedLuaFiles.clear();
		SCRIPTING::_mods.clear();
		SCRIPTING::_plugins.clear();
	}

	void SCRIPTING::setConsole(rawrbox::Console* console) { SCRIPTING::_console = console; }

	// UTILS ----
	bool SCRIPTING::hasMod(const std::string& id) {
		return SCRIPTING::getMod(id) != nullptr;
	}

	rawrbox::Mod* SCRIPTING::getMod(const std::string& id) {
		const auto& fnd = std::ranges::find(SCRIPTING::_mods, id, [](const auto& mod) -> const std::string& {
			return mod->getID();
		});

		return fnd == SCRIPTING::_mods.end() ? nullptr : fnd->get();
	}

	rawrbox::Mod* SCRIPTING::getMod(lua_State* L) {
		if (L == nullptr) return nullptr;
		return rawrbox::Mod::fromState(L);
	}

	const std::vector<std::unique_ptr<rawrbox::Mod>>& SCRIPTING::getMods() { return SCRIPTING::_mods; }
	std::vector<std::string> SCRIPTING::getModsIds() {
		std::vector<std::string> modNames = {};

		modNames.reserve(SCRIPTING::_mods.size());
		for (const auto& mod : SCRIPTING::_mods) {
			modNames.push_back(mod->getID());
		}

		return modNames;
	}

	bool SCRIPTING::hotReloadEnabled() { return SCRIPTING::_hotReloadEnabled; }
	bool SCRIPTING::hasLibrary(uint32_t library) { return (SCRIPTING::_libraries & library) == library; }
	bool SCRIPTING::isLuaFileMounted(const std::string& path) {
		const std::filesystem::path filePath = path;

		for (auto& pt : SCRIPTING::_loadedLuaFiles) {
			const auto& fnd = std::ranges::find(pt.second, filePath) != pt.second.end();
			if (fnd) return true;
		};

		return false;
	}

#ifdef RAWRBOX_SCRIPTING_WORKSHOP_MODDING
	std::vector<rawrbox::Mod*> SCRIPTING::getModsByWorkshopType(uint32_t type) {
		std::vector<rawrbox::Mod*> mods = {};
		for (auto& mod : SCRIPTING::_mods) {
			if (mod->getWorkshopType() == type) mods.push_back(mod.get());
		}

		return mods;
	}

	rawrbox::Mod* SCRIPTING::getModByWorkshopId(uint64_t id) {
		for (const auto& mod : SCRIPTING::_mods) {
			if (mod->getWorkshopId() == id) return mod.get();
		}

		return nullptr;
	}
#endif
	// -----
} // namespace rawrbox