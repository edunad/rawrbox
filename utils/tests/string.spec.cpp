#include <rawrbox/utils/string.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("StrUtils should behave as expected", "[rawrBox::StrUtils]") {
	SECTION("rawrBox::StrUtils::toLower") {
		REQUIRE(rawrBox::StrUtils::toLower("AvI STOP PLayING BAD GAmES") == "avi stop playing bad games");
	}

	SECTION("rawrBox::StrUtils::toUpper") {
		REQUIRE(rawrBox::StrUtils::toUpper("AvI STOP PLayING BAD GAmES") == "AVI STOP PLAYING BAD GAMES");
	}

	SECTION("rawrBox::StrUtils::isNumeric") {
		REQUIRE(rawrBox::StrUtils::isNumeric("A3B4") == false);
	}

	SECTION("rawrBox::StrUtils::extractNumbers") {
		REQUIRE(rawrBox::StrUtils::extractNumbers("A3B4") == "34");
	}

	SECTION("rawrBox::StrUtils::split") {
		auto spl = rawrBox::StrUtils::split("a|bC|C", '|');
		auto spl2 = rawrBox::StrUtils::split("abc", '|');

		REQUIRE(spl[0] == "a");
		REQUIRE(spl[1] == "bC");
		REQUIRE(spl[2] == "C");

		REQUIRE(spl2[0] == "abc");
	}
}
