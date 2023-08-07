#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Vector4 should behave as expected", "[rawrbox::Vector4]") {
	rawrbox::Vector4 q = rawrbox::Vector4{3.F, 1.F, 7.F, 10.F};

	SECTION("rawrbox::Vector4") {
		REQUIRE(q.x == 3.F);
		REQUIRE(q.y == 1.F);
		REQUIRE(q.z == 7.F);
		REQUIRE(q.w == 10.F);
	}

	SECTION("rawrbox::Vector3 to rawrbox::Vector4") {
		rawrbox::Vector4 q2 = rawrbox::Vector4{rawrbox::Vector3{3, 1, 7}, 10.F};
		REQUIRE(q2.x == 3.F);
		REQUIRE(q2.y == 1.F);
		REQUIRE(q2.z == 7.F);
		REQUIRE(q2.w == 10.F);
	}

	SECTION("rawrbox::Vector2 to rawrbox::Vector4") {
		rawrbox::Vector4 q2 = rawrbox::Vector4{rawrbox::Vector2{3, 1}, 7.F, 10.F};
		REQUIRE(q2.x == 3.F);
		REQUIRE(q2.y == 1.F);
		REQUIRE(q2.z == 7.F);
		REQUIRE(q2.w == 10.F);
	}

	SECTION("rawrbox::Vector4::length") {
		REQUIRE(q.length() == 12.60952F);
	}

	SECTION("rawrbox::Vector4::normalized") {
		auto n = q.normalized();

		REQUIRE_THAT(n.x, Catch::Matchers::WithinAbs(0.23792F, 0.0001F));
		REQUIRE_THAT(n.y, Catch::Matchers::WithinAbs(0.07931F, 0.0001F));
		REQUIRE_THAT(n.z, Catch::Matchers::WithinAbs(0.55514F, 0.0001F));
		REQUIRE_THAT(n.w, Catch::Matchers::WithinAbs(0.79305F, 0.0001F));
	}

	SECTION("rawrbox::Vector4::lerp") {
		rawrbox::Vector4f o = rawrbox::Vector4f{1.F, 1.F, 0.F, 10.F};
		auto n = q.lerp(o, 0.5F);

		REQUIRE_THAT(n.x, Catch::Matchers::WithinAbs(0.1847F, 0.0001F));
		REQUIRE_THAT(n.y, Catch::Matchers::WithinAbs(0.09235F, 0.0001F));
		REQUIRE_THAT(n.z, Catch::Matchers::WithinAbs(0.32323F, 0.0001F));
		REQUIRE_THAT(n.w, Catch::Matchers::WithinAbs(0.92351F, 0.0001F));
	}

	SECTION("rawrbox::Vector4::toEuler") {
		auto n = q.toEuler();

		REQUIRE_THAT(n.x, Catch::Matchers::WithinAbs(1.82212F, 0.0001F));
		REQUIRE_THAT(n.y, Catch::Matchers::WithinAbs(-1.5708F, 0.0001F));
		REQUIRE_THAT(n.z, Catch::Matchers::WithinAbs(2.16666F, 0.0001F));
	}

	SECTION("rawrbox::Vector4::toQuat") {
		auto n2 = rawrbox::Vector4f::toQuat({0.4F, 0.12F, 0.8F});

		REQUIRE_THAT(n2.x, Catch::Matchers::WithinAbs(0.15977F, 0.0001F));
		REQUIRE_THAT(n2.y, Catch::Matchers::WithinAbs(-0.13136F, 0.0001F));
		REQUIRE_THAT(n2.z, Catch::Matchers::WithinAbs(0.39194F, 0.0001F));
		REQUIRE_THAT(n2.w, Catch::Matchers::WithinAbs(0.89644F, 0.0001F));
	}
}
