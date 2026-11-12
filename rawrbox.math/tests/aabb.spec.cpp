#include <rawrbox/math/aabb.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("AABB should behave as expected", "[rawrbox::AABB]") {
	rawrbox::AABB aabb = {0.F, 0.F, 10.F, 10.F};

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
		REQUIRE(aabb.contains(rawrbox::Vector2f(-1.F, -1.F)) == false);
		REQUIRE(aabb.contains(rawrbox::Vector2f(1.5F, 1.5F)) == true);
		REQUIRE(aabb.contains(rawrbox::Vector2f(8.F, 8.F)) == true);
		REQUIRE(aabb.contains(rawrbox::Vector2f(12.F, 12.F)) == false);

		REQUIRE(aabb.contains(rawrbox::AABB{5.F, 5.F, 10.F, 10.F}) == true);
		REQUIRE(aabb.contains(rawrbox::AABB{10.F, 2.F, 4.F, 4.F}) == true);
		REQUIRE(aabb.contains(rawrbox::AABB{11.F, 2.F, 4.F, 4.F}) == false);
	}

	SECTION("rawrbox::AABB::combine") {
		rawrbox::AABBu area = {2, 2, 4, 4};

		area.combine({8, 1, 2, 2});
		REQUIRE(area == rawrbox::AABBu{2, 1, 8, 5});

		area.combine({3, 3, 1, 1});
		REQUIRE(area == rawrbox::AABBu{2, 1, 8, 5});

		aabb.combine({-5.F, 4.F, 2.F, 2.F});
		REQUIRE(aabb == rawrbox::AABB{-5.F, 0.F, 15.F, 10.F});
	}

	SECTION("rawrbox::AABB::mask") {
		auto mask = aabb.mask({2, 2, 5, 5});

		REQUIRE(mask.contains(rawrbox::Vector2f(-1.F, -1.F)) == false);
		REQUIRE(mask.contains(rawrbox::Vector2f(1.5F, 1.5F)) == false);
		REQUIRE(mask.contains(rawrbox::Vector2f(3.F, 3.F)) == true);
		REQUIRE(mask.contains(rawrbox::Vector2f(8.F, 8.F)) == false);
	}
}
