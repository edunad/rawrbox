#include <rawrbox/math/aabb.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("AABB should behave as expected", "[rawrbox::AABB]") {
	rawrbox::AABB aabb = rawrbox::AABB{0.F, 0.F, 10.F, 10.F};

	SECTION("rawrbox::AABB") {
		REQUIRE(aabb.top() == 0.F);
		REQUIRE(aabb.left() == 0.F);
		REQUIRE(aabb.bottom() == 10.F);
		REQUIRE(aabb.right() == 10.F);
	}

	SECTION("rawrbox::AABB::surfaceArea") {
		REQUIRE(aabb.surfaceArea() == 100.F);
	}

	SECTION("rawrbox::AABB::contains") {
		REQUIRE(aabb.contains({-1.F, -1.F}) == false);
		REQUIRE(aabb.contains({1.5F, 1.5F}) == true);
		REQUIRE(aabb.contains({8.F, 8.F}) == true);
		REQUIRE(aabb.contains({12.F, 12.F}) == false);
	}

	SECTION("rawrbox::AABB::mask") {
		auto mask = aabb.mask({2, 2, 5, 5});

		REQUIRE(mask.contains({-1.F, -1.F}) == false);
		REQUIRE(mask.contains({1.5F, 1.5F}) == false);
		REQUIRE(mask.contains({3.F, 3.F}) == true);
		REQUIRE(mask.contains({8.F, 8.F}) == false);
	}
}
