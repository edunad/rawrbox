#include <rawrbox/math/quaternion.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Quaternion should behave as expected", "[rawrBox::Quaternion]") {
	rawrBox::Quaternion q = rawrBox::Quaternion{3.F, 1.F, 7.F, 10.F};

	SECTION("rawrBox::Quaternion") {
		REQUIRE(q.w == 3.F);
		REQUIRE(q.x == 1.F);
		REQUIRE(q.y == 7.F);
		REQUIRE(q.z == 10.F);
	}

	SECTION("rawrBox::Quaternion::length") {
		REQUIRE(q.length() == 12.60952F);
	}

	SECTION("rawrBox::Quaternion::normalized") {
		auto n = q.normalized();

		REQUIRE_THAT(n.w, Catch::Matchers::WithinAbs(0.23792F, 0.0001F));
		REQUIRE_THAT(n.x, Catch::Matchers::WithinAbs(0.07931F, 0.0001F));
		REQUIRE_THAT(n.y, Catch::Matchers::WithinAbs(0.55514F, 0.0001F));
		REQUIRE_THAT(n.z, Catch::Matchers::WithinAbs(0.79305F, 0.0001F));
	}

	SECTION("rawrBox::Quaternion::lerp") {
		rawrBox::Quaternion o = rawrBox::Quaternion{1.F, 1.F, 0.F, 10.F};
		auto n = q.lerp(o, 0.5F);

		REQUIRE_THAT(n.w, Catch::Matchers::WithinAbs(0.1847F, 0.0001F));
		REQUIRE_THAT(n.x, Catch::Matchers::WithinAbs(0.09235F, 0.0001F));
		REQUIRE_THAT(n.y, Catch::Matchers::WithinAbs(0.32323F, 0.0001F));
		REQUIRE_THAT(n.z, Catch::Matchers::WithinAbs(0.92351F, 0.0001F));
	}
}
