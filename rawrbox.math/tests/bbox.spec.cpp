#include <rawrbox/math/bbox.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("BBOX should behave as expected", "[rawrbox::BBOX]") {
	SECTION("rawrbox::BBOX") {
		rawrbox::BBOX bbox(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));

		REQUIRE(bbox.min == rawrbox::Vector3f(0, 0, 0));
		REQUIRE(bbox.max == rawrbox::Vector3f(1, 1, 1));
		REQUIRE(bbox.size == rawrbox::Vector3f(1, 1, 1));
	}

	SECTION("rawrbox::BBOX::isEmpty") {
		rawrbox::BBOX bbox1;
		rawrbox::BBOX bbox2(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));

		REQUIRE(bbox1.isEmpty() == true);
		REQUIRE(bbox2.isEmpty() == false);
	}

	SECTION("rawrbox::BBOX::size") {
		rawrbox::BBOX bbox(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 2, 3), rawrbox::Vector3f(1, 2, 3));

		REQUIRE(bbox.size == rawrbox::Vector3f(1, 2, 3));
	}

	SECTION("rawrbox::BBOX::expand") {
		rawrbox::BBOX bbox(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));
		bbox.expand(rawrbox::Vector3f(2, 2, 2));

		REQUIRE(bbox.min == rawrbox::Vector3f(0, 0, 0));
		REQUIRE(bbox.max == rawrbox::Vector3f(2, 2, 2));
		REQUIRE(bbox.size == rawrbox::Vector3f(2, 2, 2));
	}

	SECTION("rawrbox::BBOX::contains") {
		rawrbox::BBOX bbox(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));

		REQUIRE(bbox.contains(rawrbox::Vector3f(0.5F, 0.5F, 0.5F)) == true);
		REQUIRE(bbox.contains(rawrbox::Vector3f(1.5F, 1.5F, 1.5F)) == false);
	}

	SECTION("rawrbox::BBOX::combine") {
		rawrbox::BBOX bbox1(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));
		rawrbox::BBOX bbox2(rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(2, 2, 2), rawrbox::Vector3f(1, 1, 1));
		bbox1.combine(bbox2);

		REQUIRE(bbox1.min == rawrbox::Vector3f(0, 0, 0));
		REQUIRE(bbox1.max == rawrbox::Vector3f(2, 2, 2));
		REQUIRE(bbox1.size == rawrbox::Vector3f(2, 2, 2));

		rawrbox::BBOX bbox3(rawrbox::Vector3f(2, 2, 2), rawrbox::Vector3f(3, 3, 3), rawrbox::Vector3f(1, 1, 1));
		rawrbox::BBOX bbox4(rawrbox::Vector3f(5, 5, 5), rawrbox::Vector3f(6, 6, 6), rawrbox::Vector3f(1, 1, 1));
		bbox3.combine(bbox4);

		REQUIRE(bbox3.min == rawrbox::Vector3f(2, 2, 2));
		REQUIRE(bbox3.max == rawrbox::Vector3f(6, 6, 6));
		REQUIRE(bbox3.size == rawrbox::Vector3f(4, 4, 4));
	}

	SECTION("rawrbox::BBOX::infinite") {
		auto bbox = rawrbox::BBOX::infinite();
		rawrbox::BBOX finite(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));

		REQUIRE(bbox.isInfinite());
		REQUIRE_FALSE(finite.isInfinite());
		REQUIRE(bbox.contains(rawrbox::Vector3f(1000000.F, -1000000.F, 0.F)));
		rawrbox::Matrix4x4 translate;
		translate.translate({5.F, 0.F, 0.F});
		REQUIRE(bbox.transform(translate).isInfinite());
	}

	SECTION("rawrbox::BBOX::transform") {
		rawrbox::BBOX bbox(rawrbox::Vector3f(-1, -1, -1), rawrbox::Vector3f(1, 2, 3), rawrbox::Vector3f(2, 3, 4));

		rawrbox::Matrix4x4 translate;
		translate.translate({10.F, 0.F, -5.F});

		auto moved = bbox.transform(translate);
		REQUIRE(moved.min == rawrbox::Vector3f(9, -1, -6));
		REQUIRE(moved.max == rawrbox::Vector3f(11, 2, -2));
		REQUIRE(moved.size == rawrbox::Vector3f(2, 3, 4));

		rawrbox::Matrix4x4 scale;
		scale.scale({2.F, 2.F, 2.F});

		auto scaled = bbox.transform(scale);
		REQUIRE(scaled.min == rawrbox::Vector3f(-2, -2, -2));
		REQUIRE(scaled.max == rawrbox::Vector3f(2, 4, 6));
		REQUIRE(scaled.size == rawrbox::Vector3f(4, 6, 8));
	}

	SECTION("rawrbox::BBOX::inFrustum") {
		rawrbox::Matrix4x4 identity;

		REQUIRE(rawrbox::BBOX({-0.5F, -0.5F, -0.5F}, {0.5F, 0.5F, 0.5F}, {1, 1, 1}).inFrustum(identity));
		REQUIRE(rawrbox::BBOX({0.5F, 0.5F, 0.5F}, {3, 3, 3}, {2.5F, 2.5F, 2.5F}).inFrustum(identity));
		REQUIRE(rawrbox::BBOX({-3, -3, -3}, {3, 3, 3}, {6, 6, 6}).inFrustum(identity));
		REQUIRE_FALSE(rawrbox::BBOX({2, -0.5F, -0.5F}, {3, 0.5F, 0.5F}, {1, 1, 1}).inFrustum(identity));
		REQUIRE_FALSE(rawrbox::BBOX({-0.5F, -0.5F, 1.5F}, {0.5F, 0.5F, 2}, {1, 1, 0.5F}).inFrustum(identity));
		REQUIRE(rawrbox::BBOX::infinite().inFrustum(identity));

		const auto proj = rawrbox::Matrix4x4::mtxProj(90.F, 1.F, 0.1F, 100.F);
		const float front = rawrbox::Matrix4x4::MTX_RIGHT_HANDED ? -1.F : 1.F;

		REQUIRE(rawrbox::BBOX({-1, -1, front * 10.F - 1.F}, {1, 1, front * 10.F + 1.F}, {2, 2, 2}).inFrustum(proj));
		REQUIRE_FALSE(rawrbox::BBOX({-1, -1, -front * 10.F - 1.F}, {1, 1, -front * 10.F + 1.F}, {2, 2, 2}).inFrustum(proj));
		REQUIRE_FALSE(rawrbox::BBOX({30, -1, front * 10.F - 1.F}, {32, 1, front * 10.F + 1.F}, {2, 2, 2}).inFrustum(proj));
		REQUIRE_FALSE(rawrbox::BBOX({-1, -1, front * 200.F - 1.F}, {1, 1, front * 200.F + 1.F}, {2, 2, 2}).inFrustum(proj));
	}

	SECTION("rawrbox::BBOX::operator==") {
		rawrbox::BBOX bbox1(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));
		rawrbox::BBOX bbox2(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));
		rawrbox::BBOX bbox3(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(2, 2, 2), rawrbox::Vector3f(2, 2, 2));
		rawrbox::BBOX bbox4(rawrbox::Vector3f(5, 5, 5), rawrbox::Vector3f(6, 6, 6), rawrbox::Vector3f(1, 1, 1));

		REQUIRE(bbox1 == bbox2);
		REQUIRE(bbox1 != bbox3);
		REQUIRE(bbox1 != bbox4);
	}

	SECTION("rawrbox::BBOX::operator*") {
		rawrbox::BBOX bbox(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));
		rawrbox::BBOX scaledBbox = bbox * 2.0F;

		REQUIRE(scaledBbox.min == rawrbox::Vector3f(0, 0, 0));
		REQUIRE(scaledBbox.max == rawrbox::Vector3f(2, 2, 2));
		REQUIRE(scaledBbox.size == rawrbox::Vector3f(2, 2, 2));
	}
}
