#include <rawrbox/math/utils/math.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

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

	SECTION("rawrbox::MathUtils::round") {
		REQUIRE_THAT(rawrbox::MathUtils::round(0.0332F), Catch::Matchers::WithinAbs(0.03F, 0.0001F));
		REQUIRE_THAT(rawrbox::MathUtils::round(0.432F, 1), Catch::Matchers::WithinAbs(0.4F, 0.0001F));
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
		REQUIRE(rawrbox::MathUtils::nextPow2(32) == 32);
		REQUIRE(rawrbox::MathUtils::nextPow2(43) == 64);
		REQUIRE(rawrbox::MathUtils::nextPow2(34353453) == 67108864);
		REQUIRE(rawrbox::MathUtils::nextPow2(1) == 1);
	}

	SECTION("rawrbox::MathUtils::angleDistance") {
		REQUIRE(rawrbox::MathUtils::angleDistance(179, -180) == 1);
		REQUIRE(rawrbox::MathUtils::angleDistance(0, -90) == 90);
		REQUIRE(rawrbox::MathUtils::angleDistance(-90, 90) == 180);
	}

	SECTION("rawrbox::MathUtils::toRad") {
		rawrbox::Vector3f t = {90, 0, 180};
		rawrbox::Vector3f tRad = rawrbox::MathUtils::toRad(t);

		rawrbox::Vector2f t2 = {90, 180};
		rawrbox::Vector2f t2Rad = rawrbox::MathUtils::toRad(t2);

		REQUIRE_THAT(tRad.x, Catch::Matchers::WithinAbs(1.5708F, 0.0001F));
		REQUIRE_THAT(tRad.y, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(tRad.z, Catch::Matchers::WithinAbs(3.14159F, 0.0001F));

		REQUIRE_THAT(t2Rad.x, Catch::Matchers::WithinAbs(1.5708F, 0.0001F));
		REQUIRE_THAT(t2Rad.y, Catch::Matchers::WithinAbs(3.14159F, 0.0001F));
	}

	SECTION("rawrbox::MathUtils::toDeg") {
		rawrbox::Vector3f t = {1.5708F, 0, 3.14159F};
		rawrbox::Vector3f tDeg = rawrbox::MathUtils::toDeg(t);

		rawrbox::Vector2f t2 = {1.5708F, 3.14159F};
		rawrbox::Vector2f t2Deg = rawrbox::MathUtils::toDeg(t2);

		REQUIRE_THAT(tDeg.x, Catch::Matchers::WithinAbs(90.00021F, 0.0001F));
		REQUIRE_THAT(tDeg.y, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(tDeg.z, Catch::Matchers::WithinAbs(179.99985F, 0.0001F));

		REQUIRE_THAT(t2Deg.x, Catch::Matchers::WithinAbs(90.00021F, 0.0001F));
		REQUIRE_THAT(t2Deg.y, Catch::Matchers::WithinAbs(179.99985F, 0.0001F));
	}
}
