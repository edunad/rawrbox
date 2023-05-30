#include <rawrbox/math/vector2.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Vector2 should behave as expected", "[rawrbox::Vector2]") {

	SECTION("rawrbox::Vector2") {
		rawrbox::Vector2i col = rawrbox::Vector2i{1, 5};
		rawrbox::Vector2i col2 = rawrbox::Vector2i::zero();
		rawrbox::Vector2i col3 = rawrbox::Vector2i::one();

		REQUIRE(col.x == 1);
		REQUIRE(col.y == 5);

		REQUIRE(col2.x == 0);
		REQUIRE(col2.y == 0);

		REQUIRE(col3.x == 1);
		REQUIRE(col3.y == 1);
	}

	SECTION("rawrbox::Vector2::cast") {
		rawrbox::Vector2i col = rawrbox::Vector2i{1, 5};
		auto clCast = col.cast<float>();

		REQUIRE(clCast.x == 1.F);
		REQUIRE(clCast.y == 5.F);
	}

	SECTION("rawrbox::Vector2::yx") {
		rawrbox::Vector2i col = rawrbox::Vector2i{1, 5}.yx();

		REQUIRE(col.x == 5);
		REQUIRE(col.y == 1);
	}

	SECTION("rawrbox::Vector2::lerp") {
		rawrbox::Vector2i l = rawrbox::Vector2i(255, 0).lerp({255, 255}, 0.5F);

		REQUIRE(l.x == 255);
		REQUIRE(l.y == 127);
	}

	SECTION("rawrbox::Vector2::dot") {
		float dot = rawrbox::Vector2f(1.F, 0).dot({10.F, 10.F});
		REQUIRE(dot == 10.F);
	}

	SECTION("rawrbox::Vector2::cross") {
		float dot = rawrbox::Vector2f(1.F, 0).cross({10.F, 10.F});
		REQUIRE(dot == 10.F);
	}

	SECTION("rawrbox::Vector2::normalized") {
		auto norm = rawrbox::Vector2f(5.F, 3.F).normalized();

		REQUIRE_THAT(norm.x, Catch::Matchers::WithinAbs(0.85749F, 0.0001F));
		REQUIRE_THAT(norm.y, Catch::Matchers::WithinAbs(0.5145F, 0.0001F));
	}

	SECTION("rawrbox::Vector2::length") {
		auto l = rawrbox::Vector2f(5.F, 3.F).length();
		REQUIRE_THAT(l, Catch::Matchers::WithinAbs(5.83095F, 0.0001F));
	}

	SECTION("rawrbox::Vector2::angle") {
		auto l = rawrbox::Vector2f(5.F, 3.F).angle({0, 5});
		REQUIRE_THAT(l, Catch::Matchers::WithinAbs(-1.19029F, 0.0001F));
	}

	SECTION("rawrbox::Vector2::distance") {
		auto l = rawrbox::Vector2i(5, 0).distance({5, 5});
		REQUIRE(l == 5);
	}

	SECTION("rawrbox::Vector2::abs") {
		auto dot = rawrbox::Vector2i(-23, -4).abs();

		REQUIRE(dot.x == 23);
		REQUIRE(dot.y == 4);
	}

	SECTION("rawrbox::Vector2::floor") {
		auto dot = rawrbox::Vector2f(-0.52F, -1.F).floor();

		REQUIRE(dot.x == -1.F);
		REQUIRE(dot.y == -1.F);
	}

	SECTION("rawrbox::Vector2::round") {
		auto dot = rawrbox::Vector2f(-0.51F, -1.F).round();

		REQUIRE(dot.x == -1.F);
		REQUIRE(dot.y == -1.F);
	}

	SECTION("rawrbox::Vector2::ceil") {
		auto dot = rawrbox::Vector2f(-0.51F, -1.F).ceil();

		REQUIRE(dot.x == -0.F);
		REQUIRE(dot.y == -1.F);
	}

	SECTION("rawrbox::Vector2::clamp") {
		auto dot = rawrbox::Vector2f(200.F, -100.F).clamp(0.F, 100.F);

		REQUIRE(dot.x == 100.F);
		REQUIRE(dot.y == 0.F);
	}
}
