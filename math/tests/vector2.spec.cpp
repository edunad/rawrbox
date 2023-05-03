#include <rawrbox/math/vector2.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Vector2 should behave as expected", "[rawrBox::Vector2]") {

	SECTION("rawrBox::Vector2") {
		rawrBox::Vector2i col = rawrBox::Vector2i{1, 5};
		rawrBox::Vector2i col2 = rawrBox::Vector2i::zero();
		rawrBox::Vector2i col3 = rawrBox::Vector2i::one();

		REQUIRE(col.x == 1);
		REQUIRE(col.y == 5);

		REQUIRE(col2.x == 0);
		REQUIRE(col2.y == 0);

		REQUIRE(col3.x == 1);
		REQUIRE(col3.y == 1);
	}

	SECTION("rawrBox::Vector2::cast") {
		rawrBox::Vector2i col = rawrBox::Vector2i{1, 5};
		auto clCast = col.cast<float>();

		REQUIRE(clCast.x == 1.F);
		REQUIRE(clCast.y == 5.F);
	}

	SECTION("rawrBox::Vector2::yx") {
		rawrBox::Vector2i col = rawrBox::Vector2i{1, 5}.yx();

		REQUIRE(col.x == 5);
		REQUIRE(col.y == 1);
	}

	SECTION("rawrBox::Vector2::lerp") {
		rawrBox::Vector2i l = rawrBox::Vector2i(255, 0).lerp({255, 255}, 0.5F);

		REQUIRE(l.x == 255);
		REQUIRE(l.y == 127);
	}

	SECTION("rawrBox::Vector2::dot") {
		float dot = rawrBox::Vector2f(1.F, 0).dot({10.F, 10.F});
		REQUIRE(dot == 10.F);
	}

	SECTION("rawrBox::Vector2::cross") {
		float dot = rawrBox::Vector2f(1.F, 0).cross({10.F, 10.F});
		REQUIRE(dot == 10.F);
	}

	SECTION("rawrBox::Vector2::normalized") {
		auto norm = rawrBox::Vector2f(5.F, 3.F).normalized();

		REQUIRE_THAT(norm.x, Catch::Matchers::WithinAbs(0.85749F, 0.0001F));
		REQUIRE_THAT(norm.y, Catch::Matchers::WithinAbs(0.5145F, 0.0001F));
	}

	SECTION("rawrBox::Vector2::length") {
		auto l = rawrBox::Vector2f(5.F, 3.F).length();
		REQUIRE_THAT(l, Catch::Matchers::WithinAbs(5.83095F, 0.0001F));
	}

	SECTION("rawrBox::Vector2::angle") {
		auto l = rawrBox::Vector2f(5.F, 3.F).angle({0, 5});
		REQUIRE_THAT(l, Catch::Matchers::WithinAbs(-1.9513F, 0.0001F));
	}

	SECTION("rawrBox::Vector2::distance") {
		auto l = rawrBox::Vector2i(5, 0).distance({5, 5});
		REQUIRE(l == 5);
	}

	SECTION("rawrBox::Vector2::abs") {
		auto dot = rawrBox::Vector2i(-23, -4).abs();

		REQUIRE(dot.x == 23);
		REQUIRE(dot.y == 4);
	}

	SECTION("rawrBox::Vector2::floor") {
		auto dot = rawrBox::Vector2f(-0.52F, -1.F).floor();

		REQUIRE(dot.x == -1.F);
		REQUIRE(dot.y == -1.F);
	}

	SECTION("rawrBox::Vector2::round") {
		auto dot = rawrBox::Vector2f(-0.51F, -1.F).round();

		REQUIRE(dot.x == -1.F);
		REQUIRE(dot.y == -1.F);
	}

	SECTION("rawrBox::Vector2::ceil") {
		auto dot = rawrBox::Vector2f(-0.51F, -1.F).ceil();

		REQUIRE(dot.x == -0.F);
		REQUIRE(dot.y == -1.F);
	}

	SECTION("rawrBox::Vector2::clamp") {
		auto dot = rawrBox::Vector2f(200.F, -100.F).clamp(0.F, 100.F);

		REQUIRE(dot.x == 100.F);
		REQUIRE(dot.y == 0.F);
	}
}
