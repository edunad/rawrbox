#include <rawrbox/scripting/scripting.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Scripting should behave as expected", "[rawrbox::Scripting]") {
	std::unique_ptr<rawrbox::Scripting> script = std::make_unique<rawrbox::Scripting>();

	SECTION("rawrbox::Scripting::init") {
		REQUIRE_NOTHROW(script->init());

		REQUIRE(script->getMods().empty() == true);
		REQUIRE(script->getModsIds().empty() == true);

		REQUIRE(static_cast<bool>(script->getLua().script("return true")) == true);

		REQUIRE(script->isLuaFileMounted("./test.lua") == false);

		REQUIRE_NOTHROW(script->getLua().script("local t = Vector2:new(23, 43)"));
		REQUIRE_NOTHROW(script->getLua().script("local t = Vector3:new(23, 43, 53)"));
		REQUIRE_NOTHROW(script->getLua().script("local t = Vector4:new(23, 43, 53, 12)"));
		REQUIRE_NOTHROW(script->getLua().script("local t = Color:new(255, 0, 0, 255)"));
		REQUIRE_NOTHROW(script->getLua().script("local t = Matrix:new()"));

		REQUIRE_THROWS(script->getLua().script("local t = Vector6:new(23, 43)"));
	}

	SECTION("rawrbox::Scripting::load") {
		REQUIRE_THROWS(script->load());
	}
}
