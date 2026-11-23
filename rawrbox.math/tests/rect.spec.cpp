#include <rawrbox/math/rect.hpp>

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("Rect should behave as expected", "[rawrbox::Rect]") {
	SECTION("rawrbox::Rect") {
		rawrbox::Recti rect(1, 2, 3, 4);

		REQUIRE(rect.min == rawrbox::Vector2i(1, 2));
		REQUIRE(rect.max == rawrbox::Vector2i(3, 4));
		REQUIRE_FALSE(rect.isEmpty());
	}

	SECTION("rawrbox::Rect::empty") {
		auto rect = rawrbox::Recti::empty();
		rawrbox::Recti flipped(5, 0, 4, 2);

		REQUIRE(rect.isEmpty());
		REQUIRE(flipped.isEmpty());
		REQUIRE(rawrbox::Recti(2, 2, 2, 2).isEmpty() == false);
	}

	SECTION("rawrbox::Rect::contains") {
		rawrbox::Recti rect(1, 2, 3, 4);

		REQUIRE(rect.contains({1, 2}));
		REQUIRE(rect.contains({3, 4}));
		REQUIRE(rect.contains({2, 3}));
		REQUIRE_FALSE(rect.contains({0, 2}));
		REQUIRE_FALSE(rect.contains({3, 5}));
		REQUIRE_FALSE(rawrbox::Recti::empty().contains({0, 0}));
	}

	SECTION("rawrbox::Rect::overlaps") {
		rawrbox::Recti a(0, 0, 3, 3);

		REQUIRE(a.overlaps(rawrbox::Recti(3, 3, 5, 5)));
		REQUIRE(a.overlaps(rawrbox::Recti(1, 1, 2, 2)));
		REQUIRE_FALSE(a.overlaps(rawrbox::Recti(4, 0, 6, 3)));
		REQUIRE_FALSE(a.overlaps(rawrbox::Recti::empty()));
		REQUIRE_FALSE(rawrbox::Recti::empty().overlaps(a));
	}

	SECTION("rawrbox::Rect::combine") {
		auto rect = rawrbox::Recti::empty();

		rect.combine(rawrbox::Recti(2, 3, 4, 5));
		REQUIRE(rect == rawrbox::Recti(2, 3, 4, 5));

		rect.combine(rawrbox::Recti(0, 4, 3, 8));
		REQUIRE(rect == rawrbox::Recti(0, 3, 4, 8));

		rect.combine(rawrbox::Recti::empty());
		REQUIRE(rect == rawrbox::Recti(0, 3, 4, 8));
	}

	SECTION("rawrbox::Rect::include") {
		auto rect = rawrbox::Recti::empty();

		rect.include({2, 3});
		REQUIRE(rect == rawrbox::Recti(2, 3, 2, 3));

		rect.include({0, 5});
		REQUIRE(rect == rawrbox::Recti(0, 3, 2, 5));

		rect.include({1, 4});
		REQUIRE(rect == rawrbox::Recti(0, 3, 2, 5));
	}

	SECTION("rawrbox::Rect::clamp") {
		rawrbox::Recti bounds(0, 0, 31, 31);

		REQUIRE(rawrbox::Recti(-4, 2, 40, 10).clamp(bounds) == rawrbox::Recti(0, 2, 31, 10));
		REQUIRE(rawrbox::Recti(40, 40, 50, 50).clamp(bounds).isEmpty());
	}

	SECTION("rawrbox::Rect::expand") {
		REQUIRE(rawrbox::Recti(2, 2, 4, 4).expand(2) == rawrbox::Recti(0, 0, 6, 6));
		REQUIRE(rawrbox::Recti::empty().expand(4).isEmpty());
	}

	SECTION("rawrbox::Rect::pack") {
		REQUIRE(rawrbox::Recti(1, 2, 3, 4).pack() == 0x04030201U);
		REQUIRE(rawrbox::Recti(0, 0, 255, 255).pack() == 0xFFFF0000U);
		REQUIRE(rawrbox::Recti::empty().pack() == 0x0000FFFFU);
	}

	SECTION("rawrbox::Rect::points") {
		std::vector<rawrbox::Vector2i> points = {};
		for (const auto& cell : rawrbox::Recti(1, 2, 2, 3).points()) {
			points.push_back(cell);
		}

		REQUIRE(points == std::vector<rawrbox::Vector2i>{{1, 2}, {2, 2}, {1, 3}, {2, 3}});

		size_t single = 0;
		for ([[maybe_unused]] const auto& cell : rawrbox::Recti(4, 4, 4, 4).points()) {
			++single;
		}

		size_t none = 0;
		for ([[maybe_unused]] const auto& cell : rawrbox::Recti::empty().points()) {
			++none;
		}

		REQUIRE(single == 1);
		REQUIRE(none == 0);
	}

	SECTION("rawrbox::Rect::operator+") {
		REQUIRE(rawrbox::Recti(1, 2, 3, 4) + rawrbox::Vector2i(2, -1) == rawrbox::Recti(3, 1, 5, 3));
		REQUIRE(rawrbox::Recti(1, 2, 3, 4) - rawrbox::Vector2i(1, 1) == rawrbox::Recti(0, 1, 2, 3));
		REQUIRE((rawrbox::Recti::empty() + rawrbox::Vector2i(5, 5)).isEmpty());
	}

	SECTION("rawrbox::Rect::operator==") {
		REQUIRE(rawrbox::Recti(1, 2, 3, 4) == rawrbox::Recti(1, 2, 3, 4));
		REQUIRE(rawrbox::Recti(1, 2, 3, 4) != rawrbox::Recti(1, 2, 3, 5));
	}
}