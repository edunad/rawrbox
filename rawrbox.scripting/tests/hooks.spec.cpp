#include <rawrbox/scripting/wrappers/hooks.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
	lua_State* makeState() {
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		rawrbox::Hooks::registerLua(L);
		return L;
	}

	void run(lua_State* L, const std::string& code) {
		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadScript(L, "test", code));
	}

	double global(lua_State* L, const char* name) {
		lua_getglobal(L, name);
		const double value = lua_tonumber(L, -1);
		lua_pop(L, 1);
		return value;
	}
} // namespace


TEST_CASE("Hooks should behave as expected", "[rawrbox::Hooks]") {
	lua_State* a = makeState();
	lua_State* b = makeState();

	run(a, "SEEN = 0; hooks.add('ping', 'a', function(v) SEEN = SEEN + (type(v) == 'table' and v.n or v) end)");
	run(b, "SEEN = 0; hooks.add('ping', 'b', function(v) SEEN = SEEN + (type(v) == 'table' and v.n or v) end)");

	SECTION("rawrbox::Hooks::call") {
		rawrbox::Hooks::call("ping", 2);
		REQUIRE(global(a, "SEEN") == 2.0);
		REQUIRE(global(b, "SEEN") == 2.0);

		run(a, "hooks.call('ping', { n = 5 })");
		REQUIRE(global(a, "SEEN") == 7.0);
		REQUIRE(global(b, "SEEN") == 7.0);
	}

	SECTION("rawrbox::Hooks::add") {
		run(a, "hooks.add('ping', 'a', function(v) SEEN = SEEN + 100 end)");
		rawrbox::Hooks::call("ping", 1);
		REQUIRE(global(a, "SEEN") == 100.0);
		REQUIRE(global(b, "SEEN") == 1.0);
	}

	SECTION("rawrbox::Hooks::call order") {
		run(a, "ORDER = ''; hooks.add('order', 'z', function() ORDER = ORDER .. 'z' end); hooks.add('order', 'm', function() ORDER = ORDER .. 'm' end)");
		rawrbox::Hooks::call("order");

		lua_getglobal(a, "ORDER");
		REQUIRE(std::string(lua_tostring(a, -1)) == "mz");
		lua_pop(a, 1);
	}

	SECTION("rawrbox::Hooks::destroy") {
		run(b, "hooks.destroy('ping', 'b')");
		REQUIRE(rawrbox::Hooks::exists("ping", "a"));
		REQUIRE_FALSE(rawrbox::Hooks::exists("ping", "b"));

		rawrbox::Hooks::call("ping", 1);
		REQUIRE(global(b, "SEEN") == 0.0);
	}

	SECTION("rawrbox::Hooks::clear") {
		rawrbox::Hooks::clear(a);
		REQUIRE_FALSE(rawrbox::Hooks::exists("ping", "a"));
		REQUIRE(rawrbox::Hooks::exists("ping", "b"));
	}

	// CLEANUP ----
	rawrbox::Hooks::clear();
	REQUIRE(rawrbox::Hooks::empty());

	lua_close(a);
	lua_close(b);
	// ------------
}