#include <rawrbox/math/utils/math.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <string>

TEST_CASE("MathUtils should behave as expected", "[rawrbox::MathUtils]") {
	SECTION("rawrbox::MathUtils::repeat") {
		REQUIRE(rawrbox::MathUtils::repeat(2, 0, 10) == 2);
		REQUIRE(rawrbox::MathUtils::repeat(12, 0, 10) == 2);
		REQUIRE(rawrbox::MathUtils::repeat(-5, 0, 10) == 5);
	}

	SECTION("rawrbox::MathUtils::pingPong") {
		REQUIRE(rawrbox::MathUtils::pingPong(2, 10) == 2);
		REQUIRE(rawrbox::MathUtils::pingPong(15, 10) == 5);
	}

	SECTION("rawrbox::MathUtils::toRad") {
		REQUIRE_THAT(rawrbox::MathUtils::toRad(90), Catch::Matchers::WithinAbs(1.5708F, 0.0001F));
	}

	SECTION("rawrbox::MathUtils::toDeg") {
		REQUIRE(std::round(rawrbox::MathUtils::toDeg(1.5708F)) == 90);
	}

	SECTION("rawrbox::MathUtils::lerp") {
		REQUIRE(rawrbox::MathUtils::lerp(0, 10, 0.5F) == 5.0F);
	}

	SECTION("rawrbox::MathUtils::angleLerp") {
		REQUIRE(rawrbox::MathUtils::angleLerp(90, 260, 0.5F) == 175.0F);
	}

	SECTION("rawrbox::MathUtils::angleRadLerp") {
		REQUIRE_THAT(rawrbox::MathUtils::angleRadLerp(1.5708F, 4.53786F, 0.5F), Catch::Matchers::WithinAbs(3.05433F, 0.0001F));
	}

	SECTION("rawrbox::MathUtils::nextPow2") {
		REQUIRE(rawrbox::MathUtils::nextPow2(8) == 8);
		REQUIRE(rawrbox::MathUtils::nextPow2(12) == 16);
		REQUIRE(rawrbox::MathUtils::nextPow2(1) == 1);
	}
}
