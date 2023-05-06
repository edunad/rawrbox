#include <rawrbox/utils/string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("StrUtils should behave as expected", "[rawrbox::StrUtils]") {
	SECTION("rawrbox::StrUtils::toLower") {
		REQUIRE(rawrbox::StrUtils::toLower("AvI STOP PLayING BAD GAmES") == "avi stop playing bad games");
	}

	SECTION("rawrbox::StrUtils::toUpper") {
		REQUIRE(rawrbox::StrUtils::toUpper("AvI STOP PLayING BAD GAmES") == "AVI STOP PLAYING BAD GAMES");
	}

	SECTION("rawrbox::StrUtils::isNumeric") {
		REQUIRE(rawrbox::StrUtils::isNumeric("A3B4") == false);
	}

	SECTION("rawrbox::StrUtils::extractNumbers") {
		REQUIRE(rawrbox::StrUtils::extractNumbers("A3B4") == "34");
	}

	SECTION("rawrbox::StrUtils::split") {
		auto spl = rawrbox::StrUtils::split("a|bC|C", '|');
		auto spl2 = rawrbox::StrUtils::split("abc", '|');

		REQUIRE(spl[0] == "a");
		REQUIRE(spl[1] == "bC");
		REQUIRE(spl[2] == "C");

		REQUIRE(spl2[0] == "abc");
	}
}
