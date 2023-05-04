#include <rawrbox/math/vector3.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Vector3 should behave as expected", "[rawrBox::Vector3]") {
	SECTION("rawrBox::Vector3") {
		rawrBox::Vector3i col = rawrBox::Vector3i{1, 5, 8};
		rawrBox::Vector3i col2 = rawrBox::Vector3i::zero();
		rawrBox::Vector3i col3 = rawrBox::Vector3i::one();

		REQUIRE(col.x == 1);
		REQUIRE(col.y == 5);
		REQUIRE(col.z == 8);

		REQUIRE(col2.x == 0);
		REQUIRE(col2.y == 0);
		REQUIRE(col2.z == 0);

		REQUIRE(col3.x == 1);
		REQUIRE(col3.y == 1);
		REQUIRE(col3.z == 1);
	}

	SECTION("rawrBox::Vector3::cast") {
		rawrBox::Vector3i col = rawrBox::Vector3i{1, 5, 8};
		auto clCast = col.cast<float>();

		REQUIRE(clCast.x == 1.F);
		REQUIRE(clCast.y == 5.F);
		REQUIRE(clCast.z == 8.F);
	}

	SECTION("rawrBox::Vector3::yxz") {
		rawrBox::Vector3i col = rawrBox::Vector3i{1, 5, 8}.yxz();

		REQUIRE(col.x == 5);
		REQUIRE(col.y == 1);
		REQUIRE(col.z == 8);
	}

	SECTION("rawrBox::Vector3::zyx") {
		rawrBox::Vector3i col = rawrBox::Vector3i{1, 5, 8}.zyx();

		REQUIRE(col.x == 8);
		REQUIRE(col.y == 5);
		REQUIRE(col.z == 1);
	}

	SECTION("rawrBox::Vector3::xzy") {
		rawrBox::Vector3i col = rawrBox::Vector3i{1, 5, 8}.xzy();

		REQUIRE(col.x == 1);
		REQUIRE(col.y == 8);
		REQUIRE(col.z == 5);
	}

	SECTION("rawrBox::Vector3::lerp") {
		rawrBox::Vector3i l = rawrBox::Vector3i(255, 0, 155).lerp({255, 255, 255}, 0.5F);

		REQUIRE(l.x == 255);
		REQUIRE(l.y == 127);
		REQUIRE(l.z == 205);
	}

	SECTION("rawrBox::Vector3::dot") {
		float dot = rawrBox::Vector3f(1.F, 0, 0).dot({10.F, 10.F, 10.F});
		REQUIRE(dot == 10.F);
	}

	SECTION("rawrBox::Vector3::cross") {
		auto dot = rawrBox::Vector3f(1.F, 0, 3.F).cross({10.F, 10.F, 8.F});

		REQUIRE(dot.x == -30.0F);
		REQUIRE(dot.y == 22.0F);
		REQUIRE(dot.z == 10.F);
	}

	SECTION("rawrBox::Vector3::normalized") {
		auto norm = rawrBox::Vector3f(5.F, 3.F, 8.F).normalized();

		REQUIRE_THAT(norm.x, Catch::Matchers::WithinAbs(0.50508F, 0.0001F));
		REQUIRE_THAT(norm.y, Catch::Matchers::WithinAbs(0.30305F, 0.0001F));
		REQUIRE_THAT(norm.z, Catch::Matchers::WithinAbs(0.80812F, 0.0001F));
	}

	SECTION("rawrBox::Vector3::length") {
		auto l = rawrBox::Vector3f(5.F, 3.F, 8.F).length();
		REQUIRE_THAT(l, Catch::Matchers::WithinAbs(9.8995F, 0.0001F));
	}

	SECTION("rawrBox::Vector3::angle") {
		auto l = rawrBox::Vector3f(1.F, 1.F, 1.F).angle({1.F, 1.F, 10.F}); // Uhhhhhhh
		REQUIRE_THAT(l, Catch::Matchers::WithinAbs(46.68614F, 0.0001F));
	}

	SECTION("rawrBox::Vector3::distance") {
		auto l = rawrBox::Vector3i(0, 0, 0).distance({5, 5, 5});
		REQUIRE(l == 8);
	}

	SECTION("rawrBox::Vector3::abs") {
		auto dot = rawrBox::Vector3i(-23, -4, 9).abs();

		REQUIRE(dot.x == 23);
		REQUIRE(dot.y == 4);
		REQUIRE(dot.z == 9);
	}

	SECTION("rawrBox::Vector3::floor") {
		auto dot = rawrBox::Vector3f(-0.52F, -1.F, 0.F).floor();

		REQUIRE(dot.x == -1.F);
		REQUIRE(dot.y == -1.F);
		REQUIRE(dot.z == 0.F);
	}

	SECTION("rawrBox::Vector3::round") {
		auto dot = rawrBox::Vector3f(-0.51F, -1.F, 0.F).round();

		REQUIRE(dot.x == -1.F);
		REQUIRE(dot.y == -1.F);
		REQUIRE(dot.z == 0.F);
	}

	SECTION("rawrBox::Vector3::ceil") {
		auto dot = rawrBox::Vector3f(-0.51F, -1.F, 0.F).ceil();

		REQUIRE(dot.x == -0.F);
		REQUIRE(dot.y == -1.F);
		REQUIRE(dot.z == 0.F);
	}

	SECTION("rawrBox::Vector2::clamp") {
		auto dot = rawrBox::Vector3f(200.F, -100.F, 60.F).clamp(0.F, 100.F);

		REQUIRE(dot.x == 100.F);
		REQUIRE(dot.y == 0.F);
		REQUIRE(dot.z == 60.F);
	}
}
