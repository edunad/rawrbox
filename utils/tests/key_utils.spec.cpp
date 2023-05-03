#include <rawrbox/utils/key_utils.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("KeyUtils should behave as expected", "[rawrBox::KeyUtils]") {
	SECTION("rawrBox::KeyUtils::keyToStr") {
		REQUIRE(rawrBox::KeyUtils::keyToStr(KEY_MINUS) == "-");
		REQUIRE(rawrBox::KeyUtils::keyToStr(KEY_S) == "S");
		REQUIRE(rawrBox::KeyUtils::keyToStr(KEY_SPACE) == " ");
	}

	SECTION("rawrBox::KeyUtils::strToKey") {
		REQUIRE(rawrBox::KeyUtils::strToKey("+") == KEY_KP_ADD);
		REQUIRE(rawrBox::KeyUtils::strToKey(";") == KEY_SEMICOLON);
	}
}
