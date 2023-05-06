#include <rawrbox/utils/key_utils.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("KeyUtils should behave as expected", "[rawrbox::KeyUtils]") {
	SECTION("rawrbox::KeyUtils::keyToStr") {
		REQUIRE(rawrbox::KeyUtils::keyToStr(KEY_MINUS) == "-");
		REQUIRE(rawrbox::KeyUtils::keyToStr(KEY_S) == "S");
		REQUIRE(rawrbox::KeyUtils::keyToStr(KEY_SPACE) == " ");
	}

	SECTION("rawrbox::KeyUtils::strToKey") {
		REQUIRE(rawrbox::KeyUtils::strToKey("+") == KEY_KP_ADD);
		REQUIRE(rawrbox::KeyUtils::strToKey(";") == KEY_SEMICOLON);
	}
}
