#include <rawrbox/scripting/wrappers/hooks.hpp>
#include <rawrbox/scripting/wrappers/math/vector2.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace rawrbox {
	static lua_State* newHooksState() {
		lua_State* L = luaL_newstate();
		luaL_openlibs(L);
		rawrbox::Hooks::registerLua(L);
		rawrbox::Vector2Wrapper::registerLua(L);
		return L;
	}

	static void runHooksLua(lua_State* L, const std::string& code) {
		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadScript(L, "test", code));
	}

	static double readHooksGlobal(lua_State* L, const char* name) {
		lua_getglobal(L, name);
		const double value = lua_tonumber(L, -1);
		lua_pop(L, 1);
		return value;
	}
} // namespace rawrbox

TEST_CASE("Hooks should behave as expected", "[rawrbox::Hooks]") {
	lua_State* a = rawrbox::newHooksState();
	lua_State* b = rawrbox::newHooksState();

	rawrbox::runHooksLua(a, "SEEN = 0; hooks.add('ping', 'a', function(v) SEEN = SEEN + (type(v) == 'table' and v.n or v) end)");
	rawrbox::runHooksLua(b, "SEEN = 0; hooks.add('ping', 'b', function(v) SEEN = SEEN + (type(v) == 'table' and v.n or v) end)");

	SECTION("rawrbox::Hooks::call") {
		rawrbox::Hooks::call("ping", 2);
		REQUIRE(rawrbox::readHooksGlobal(a, "SEEN") == 2.0);
		REQUIRE(rawrbox::readHooksGlobal(b, "SEEN") == 2.0);

		rawrbox::runHooksLua(a, "hooks.call('ping', { n = 5 })");
		REQUIRE(rawrbox::readHooksGlobal(a, "SEEN") == 7.0);
		REQUIRE(rawrbox::readHooksGlobal(b, "SEEN") == 7.0);
	}

	SECTION("rawrbox::Hooks::call from lua keeps lists and userdata") {
		rawrbox::runHooksLua(b, "hooks.add('data', 'b', function(list, pos, name) GOT = {list = list, pos = pos, name = name} end)");
		rawrbox::runHooksLua(a, "hooks.add('data', 'a', function(list) list[1] = 99 end)");

		rawrbox::runHooksLua(a, "SENT = {10, 20, {30}}; hooks.call('data', SENT, Vector2i(4, -2), 'cat')");

		rawrbox::runHooksLua(b, "OK = (GOT.list[1] == 10 and GOT.list[2] == 20 and GOT.list[3][1] == 30 and GOT.pos.x == 4 and GOT.pos.y == -2 and GOT.name == 'cat') and 1 or 0");
		REQUIRE(rawrbox::readHooksGlobal(b, "OK") == 1.0);

		rawrbox::runHooksLua(a, "OK = SENT[1] == 10 and 1 or 0");
		REQUIRE(rawrbox::readHooksGlobal(a, "OK") == 1.0);
	}

	SECTION("rawrbox::Hooks::call from lua skips handlers it cannot copy to") {
		rawrbox::runHooksLua(a, "hooks.call('ping', {n = 3, fn = print})");
		REQUIRE(rawrbox::readHooksGlobal(a, "SEEN") == 0.0);
		REQUIRE(rawrbox::readHooksGlobal(b, "SEEN") == 0.0);

		rawrbox::runHooksLua(a, "hooks.call('ping', 4)");
		REQUIRE(rawrbox::readHooksGlobal(a, "SEEN") == 4.0);
		REQUIRE(rawrbox::readHooksGlobal(b, "SEEN") == 4.0);
	}

	SECTION("rawrbox::Hooks::call from lua on the stack of its handler") {
		rawrbox::runHooksLua(a, "hooks.add('self', 'a', function(list, n) GOT_SELF = list[2][1] + n end); function CALLER() hooks.call('self', {1, {9}}, 5) end");

		const int top = lua_gettop(a);
		lua_getglobal(a, "CALLER");
		REQUIRE(lua_pcall(a, 0, 0, 0) == LUA_OK);

		REQUIRE(lua_gettop(a) == top);
		REQUIRE(rawrbox::readHooksGlobal(a, "GOT_SELF") == 14.0);
	}

	SECTION("rawrbox::Hooks::add") {
		rawrbox::runHooksLua(a, "hooks.add('ping', 'a', function(v) SEEN = SEEN + 100 end)");
		rawrbox::Hooks::call("ping", 1);
		REQUIRE(rawrbox::readHooksGlobal(a, "SEEN") == 100.0);
		REQUIRE(rawrbox::readHooksGlobal(b, "SEEN") == 1.0);
	}

	SECTION("rawrbox::Hooks::call order") {
		rawrbox::runHooksLua(a, "ORDER = ''; hooks.add('order', 'z', function() ORDER = ORDER .. 'z' end); hooks.add('order', 'm', function() ORDER = ORDER .. 'm' end)");
		rawrbox::Hooks::call("order");

		lua_getglobal(a, "ORDER");
		REQUIRE(std::string(lua_tostring(a, -1)) == "mz");
		lua_pop(a, 1);
	}

	SECTION("rawrbox::Hooks::destroy") {
		rawrbox::runHooksLua(b, "hooks.destroy('ping', 'b')");
		REQUIRE(rawrbox::Hooks::exists("ping", "a"));
		REQUIRE_FALSE(rawrbox::Hooks::exists("ping", "b"));

		rawrbox::Hooks::call("ping", 1);
		REQUIRE(rawrbox::readHooksGlobal(b, "SEEN") == 0.0);
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