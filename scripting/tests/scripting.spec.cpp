#include <rawrbox/scripting/scripting.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Scripting should behave as expected", "[rawrbox::Scripting]") {
	SECTION("rawrbox::Scripting::init") {
		REQUIRE_NOTHROW(rawrbox::SCRIPTING::init());

		REQUIRE(rawrbox::SCRIPTING::getMods().empty() == true);
		REQUIRE(rawrbox::SCRIPTING::getModsIds().empty() == true);

		REQUIRE(static_cast<bool>(rawrbox::SCRIPTING::getLUA().script("return true")) == true);

		REQUIRE(rawrbox::SCRIPTING::isLuaFileMounted("./test.lua") == false);

		REQUIRE_NOTHROW(rawrbox::SCRIPTING::getLUA().script("local t = Vector2:new(23, 43)"));
		REQUIRE_NOTHROW(rawrbox::SCRIPTING::getLUA().script("local t = Vector3:new(23, 43, 53)"));
		REQUIRE_NOTHROW(rawrbox::SCRIPTING::getLUA().script("local t = Vector4:new(23, 43, 53, 12)"));
		REQUIRE_NOTHROW(rawrbox::SCRIPTING::getLUA().script("local t = Color:new(255, 0, 0, 255)"));
		REQUIRE_NOTHROW(rawrbox::SCRIPTING::getLUA().script("local t = Matrix:new()"));

		REQUIRE_THROWS(rawrbox::SCRIPTING::getLUA().script("local t = Vector6:new(23, 43)"));
	}

	SECTION("rawrbox::Scripting::load") {
		REQUIRE_THROWS(rawrbox::SCRIPTING::load());
	}
}
