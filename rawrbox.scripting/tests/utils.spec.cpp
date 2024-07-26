#include <rawrbox/scripting/utils/lua.hpp>

#include <catch2/catch_test_macros.hpp>

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
}
