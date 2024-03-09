#include <rawrbox/utils/key.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("KeyUtils should behave as expected", "[rawrbox::KeyUtils]") {
	SECTION("rawrbox::KeyUtils::keyToStr") {
		REQUIRE(rawrbox::KeyUtils::keyToStr(rawrbox::KEY_MINUS) == "-");
		REQUIRE(rawrbox::KeyUtils::keyToStr(rawrbox::KEY_S) == "S");
		REQUIRE(rawrbox::KeyUtils::keyToStr(rawrbox::KEY_SPACE) == " ");
	}

	SECTION("rawrbox::KeyUtils::strToKey") {
		REQUIRE(rawrbox::KeyUtils::strToKey("+") == rawrbox::KEY_KP_ADD);
		REQUIRE(rawrbox::KeyUtils::strToKey(";") == rawrbox::KEY_SEMICOLON);
	}
}
