#include <rawrbox/utils/qhull.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("QHull should behave as expected", "[rawrbox::QHull]") {
	SECTION("rawrbox::QHull") {
		std::vector<rawrbox::QHullPoint2D> points{{0, 1}, {1, 1}, {1, 2}, {3, 4}, {2, 4}, {0, 4}, {4, 2}, {2, 2}};
		auto convex = rawrbox::QHull::calculate2DConvex(points);

		REQUIRE(convex.size() == 5);
		REQUIRE(convex[0].x == 0.0F);
		REQUIRE(convex[0].y == 4.F);

		REQUIRE(convex[1].x == 3.F);
		REQUIRE(convex[1].y == 4.F);

		REQUIRE(convex[2].x == 1.F);
		REQUIRE(convex[2].y == 1.F);

		REQUIRE(convex[3].x == 0.0F);
		REQUIRE(convex[3].y == 1.F);

		REQUIRE(convex[4].x == 4.F);
		REQUIRE(convex[4].y == 2.F);
	}
}
