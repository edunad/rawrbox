#include <rawrbox/math/utils/math.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <string>

TEST_CASE("MathUtils should behave as expected", "[rawrBox::MathUtils]") {
	SECTION("rawrBox::MathUtils::repeat") {
		REQUIRE(rawrBox::MathUtils::repeat(2, 0, 10) == 2);
		REQUIRE(rawrBox::MathUtils::repeat(12, 0, 10) == 2);
		REQUIRE(rawrBox::MathUtils::repeat(-5, 0, 10) == 5);
	}

	SECTION("rawrBox::MathUtils::pingPong") {
		REQUIRE(rawrBox::MathUtils::pingPong(2, 10) == 2);
		REQUIRE(rawrBox::MathUtils::pingPong(15, 10) == 5);
	}

	SECTION("rawrBox::MathUtils::toRad") {
		REQUIRE_THAT(rawrBox::MathUtils::toRad(90), Catch::Matchers::WithinAbs(1.5708F, 0.0001F));
	}

	SECTION("rawrBox::MathUtils::toDeg") {
		REQUIRE(std::round(rawrBox::MathUtils::toDeg(1.5708F)) == 90);
	}

	SECTION("rawrBox::MathUtils::lerp") {
		REQUIRE(rawrBox::MathUtils::lerp(0, 10, 0.5F) == 5.0F);
	}

	SECTION("rawrBox::MathUtils::angleLerp") {
		REQUIRE(rawrBox::MathUtils::angleLerp(90, 260, 0.5F) == 175.0F);
	}

	SECTION("rawrBox::MathUtils::angleRadLerp") {
		REQUIRE_THAT(rawrBox::MathUtils::angleRadLerp(1.5708F, 4.53786F, 0.5F), Catch::Matchers::WithinAbs(3.05433F, 0.0001F));
	}

	SECTION("rawrBox::MathUtils::nextPow2") {
		REQUIRE(rawrBox::MathUtils::nextPow2(8) == 8);
		REQUIRE(rawrBox::MathUtils::nextPow2(12) == 16);
		REQUIRE(rawrBox::MathUtils::nextPow2(1) == 1);
	}
}
