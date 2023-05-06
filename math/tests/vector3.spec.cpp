#include <rawrbox/math/vector3.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Vector3 should behave as expected", "[rawrbox::Vector3]") {
	SECTION("rawrbox::Vector3") {
		rawrbox::Vector3i col = rawrbox::Vector3i{1, 5, 8};
		rawrbox::Vector3i col2 = rawrbox::Vector3i::zero();
		rawrbox::Vector3i col3 = rawrbox::Vector3i::one();

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

	SECTION("rawrbox::Vector3::cast") {
		rawrbox::Vector3i col = rawrbox::Vector3i{1, 5, 8};
		auto clCast = col.cast<float>();

		REQUIRE(clCast.x == 1.F);
		REQUIRE(clCast.y == 5.F);
		REQUIRE(clCast.z == 8.F);
	}

	SECTION("rawrbox::Vector3::yxz") {
		rawrbox::Vector3i col = rawrbox::Vector3i{1, 5, 8}.yxz();

		REQUIRE(col.x == 5);
		REQUIRE(col.y == 1);
		REQUIRE(col.z == 8);
	}

	SECTION("rawrbox::Vector3::zyx") {
		rawrbox::Vector3i col = rawrbox::Vector3i{1, 5, 8}.zyx();

		REQUIRE(col.x == 8);
		REQUIRE(col.y == 5);
		REQUIRE(col.z == 1);
	}

	SECTION("rawrbox::Vector3::xzy") {
		rawrbox::Vector3i col = rawrbox::Vector3i{1, 5, 8}.xzy();

		REQUIRE(col.x == 1);
		REQUIRE(col.y == 8);
		REQUIRE(col.z == 5);
	}

	SECTION("rawrbox::Vector3::lerp") {
		rawrbox::Vector3i l = rawrbox::Vector3i(255, 0, 155).lerp({255, 255, 255}, 0.5F);

		REQUIRE(l.x == 255);
		REQUIRE(l.y == 127);
		REQUIRE(l.z == 205);
	}

	SECTION("rawrbox::Vector3::dot") {
		float dot = rawrbox::Vector3f(1.F, 0, 0).dot({10.F, 10.F, 10.F});
		REQUIRE(dot == 10.F);
	}

	SECTION("rawrbox::Vector3::cross") {
		auto dot = rawrbox::Vector3f(1.F, 0, 3.F).cross({10.F, 10.F, 8.F});

		REQUIRE(dot.x == -30.0F);
		REQUIRE(dot.y == 22.0F);
		REQUIRE(dot.z == 10.F);
	}

	SECTION("rawrbox::Vector3::normalized") {
		auto norm = rawrbox::Vector3f(5.F, 3.F, 8.F).normalized();

		REQUIRE_THAT(norm.x, Catch::Matchers::WithinAbs(0.50508F, 0.0001F));
		REQUIRE_THAT(norm.y, Catch::Matchers::WithinAbs(0.30305F, 0.0001F));
		REQUIRE_THAT(norm.z, Catch::Matchers::WithinAbs(0.80812F, 0.0001F));
	}

	SECTION("rawrbox::Vector3::length") {
		auto l = rawrbox::Vector3f(5.F, 3.F, 8.F).length();
		REQUIRE_THAT(l, Catch::Matchers::WithinAbs(9.8995F, 0.0001F));
	}

	SECTION("rawrbox::Vector3::angle") {
		auto l = rawrbox::Vector3f(1.F, 1.F, 1.F).angle({1.F, 1.F, 10.F}); // Uhhhhhhh
		REQUIRE_THAT(l, Catch::Matchers::WithinAbs(46.68614F, 0.0001F));
	}

	SECTION("rawrbox::Vector3::distance") {
		auto l = rawrbox::Vector3i(0, 0, 0).distance({5, 5, 5});
		REQUIRE(l == 8);
	}

	SECTION("rawrbox::Vector3::abs") {
		auto dot = rawrbox::Vector3i(-23, -4, 9).abs();

		REQUIRE(dot.x == 23);
		REQUIRE(dot.y == 4);
		REQUIRE(dot.z == 9);
	}

	SECTION("rawrbox::Vector3::floor") {
		auto dot = rawrbox::Vector3f(-0.52F, -1.F, 0.F).floor();

		REQUIRE(dot.x == -1.F);
		REQUIRE(dot.y == -1.F);
		REQUIRE(dot.z == 0.F);
	}

	SECTION("rawrbox::Vector3::round") {
		auto dot = rawrbox::Vector3f(-0.51F, -1.F, 0.F).round();

		REQUIRE(dot.x == -1.F);
		REQUIRE(dot.y == -1.F);
		REQUIRE(dot.z == 0.F);
	}

	SECTION("rawrbox::Vector3::ceil") {
		auto dot = rawrbox::Vector3f(-0.51F, -1.F, 0.F).ceil();

		REQUIRE(dot.x == -0.F);
		REQUIRE(dot.y == -1.F);
		REQUIRE(dot.z == 0.F);
	}

	SECTION("rawrbox::Vector2::clamp") {
		auto dot = rawrbox::Vector3f(200.F, -100.F, 60.F).clamp(0.F, 100.F);

		REQUIRE(dot.x == 100.F);
		REQUIRE(dot.y == 0.F);
		REQUIRE(dot.z == 60.F);
	}
}
