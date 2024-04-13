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
	}

	SECTION("rawrbox::BBOX::operators") {
		rawrbox::BBOX bbox1(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));
		rawrbox::BBOX bbox2(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(1, 1, 1), rawrbox::Vector3f(1, 1, 1));
		rawrbox::BBOX bbox3(rawrbox::Vector3f(0, 0, 0), rawrbox::Vector3f(2, 2, 2), rawrbox::Vector3f(2, 2, 2));

		REQUIRE(bbox1 == bbox2);
		REQUIRE(bbox1 != bbox3);
	}
}
