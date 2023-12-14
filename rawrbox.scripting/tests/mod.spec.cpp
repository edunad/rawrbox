#include <rawrbox/scripting/mod.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Mod should behave as expected", "[rawrbox::Mod]") {
	SECTION("rawrbox::Mod::Mod") {
		rawrbox::Mod mod = {"my_mod", "./my_mod"};

		REQUIRE(mod.getFolder() == "./my_mod");
		REQUIRE(mod.getID() == "my_mod");
	}
}
