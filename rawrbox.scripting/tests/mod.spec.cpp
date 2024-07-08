#include <rawrbox/scripting/mod.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Mod should behave as expected", "[rawrbox::Mod]") {
	SECTION("rawrbox::Mod::Mod") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};

		REQUIRE(mod.getFolder() == "./my_mod");
		REQUIRE(mod.getID() == "my_mod");
	}

	SECTION("rawrbox::Mod::init") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};
		luaL_openlibs(mod.getEnvironment());

		REQUIRE_NOTHROW(mod.init());
	}

	SECTION("rawrbox::Mod::load") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};
		luaL_openlibs(mod.getEnvironment());

		REQUIRE_THROWS(mod.load());

		REQUIRE_NOTHROW(mod.init());
		REQUIRE_NOTHROW(mod.script("return true"));

		REQUIRE(!mod.call("test").has_value());
		REQUIRE_NOTHROW(mod.script("function MOD:test() return true end"));

		REQUIRE(mod.call("test").has_value());
		for (size_t i = 0; i < 10000; i++)
			REQUIRE(mod.call("test").value().wasOk());

		REQUIRE_NOTHROW(mod.gc());
	}
}
