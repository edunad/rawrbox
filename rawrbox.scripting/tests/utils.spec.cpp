#include <rawrbox/scripting/utils/lua.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

TEST_CASE("Lua Utils should behave as expected", "[rawrbox::LuaUtils]") {
	SECTION("rawrbox::LuaUtils::getContent") {
		auto test1 = rawrbox::LuaUtils::getContent("./assets/bla.png", "my_mod/");
		REQUIRE(test1.first.empty());
		REQUIRE(test1.second == "my_mod/assets/bla.png");

		auto test2 = rawrbox::LuaUtils::getContent("#/bla.png", "local_content/");
		REQUIRE(test2.first.empty());
		REQUIRE(test2.second == "assets/bla.png");

		auto test3 = rawrbox::LuaUtils::getContent("@mod_2/assets/bla.png", "my_mod/");
		REQUIRE(test3.first == "mod_2");
		REQUIRE(test3.second == "assets/bla.png");

		REQUIRE_THROWS(rawrbox::LuaUtils::getContent("", ""));
		REQUIRE_THROWS(rawrbox::LuaUtils::getContent("@/assets/bla.png", ""));
	}

	SECTION("rawrbox::LuaUtils::precompile") {
		const std::string folder = "./rawrbox-tests-lua";
		std::filesystem::create_directories(folder);

		const auto bytecode = rawrbox::LuaUtils::precompile("PRECOMPILED = 42");
		REQUIRE(bytecode.starts_with("\x1bLuau"));
		REQUIRE_THROWS(rawrbox::LuaUtils::precompile("this is not lua"));

		std::ofstream(folder + "/precompiled.lua", std::ios::binary) << bytecode;
		std::ofstream(folder + "/source.lua", std::ios::binary) << "SOURCE = 7";

		lua_State* L = luaL_newstate();
		luaL_openlibs(L);

		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadFile(L, "precompiled", folder + "/precompiled.lua"));
		REQUIRE_NOTHROW(rawrbox::LuaUtils::compileAndLoadFile(L, "source", folder + "/source.lua"));

		lua_getglobal(L, "PRECOMPILED");
		REQUIRE(lua_tonumber(L, -1) == 42);
		lua_getglobal(L, "SOURCE");
		REQUIRE(lua_tonumber(L, -1) == 7);

		lua_close(L);
		std::filesystem::remove_all(folder);
	}
}
