#include <rawrbox/math/bezier_curve.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("BezierCurve functionalities are correct", "[rawrbox::BezierCurve]") {
	std::array<rawrbox::Vector3f, 4> controlPoints = {
	    rawrbox::Vector3f(0.0F, 0.0F, 0.F),
	    rawrbox::Vector3f(1.0F, 2.0F, 0.F),
	    rawrbox::Vector3f(2.0F, 2.0F, 0.F),
	    rawrbox::Vector3f(3.0F, 0.0F, 0.F)};

	rawrbox::BezierCurve curve(controlPoints, 100);

	SECTION("rawrbox::getPoint") {
		auto point = curve.getPoint(0.5F);

		REQUIRE_THAT(point.x, Catch::Matchers::WithinAbs(1.5F, 0.01F));
		REQUIRE_THAT(point.y, Catch::Matchers::WithinAbs(2.0F, 0.01F));
		REQUIRE_THAT(point.z, Catch::Matchers::WithinAbs(0.0F, 0.01F));

		REQUIRE(curve.getPoint(0.0F) == controlPoints.front());
		REQUIRE(curve.getPoint(1.0F) == controlPoints.back());
	}

	SECTION("rawrbox::getPoint::tangent") {
		rawrbox::Vector3f tangent = {};
		curve.getPoint(0.5F, tangent);

		REQUIRE_THAT(tangent.length(), Catch::Matchers::WithinAbs(1.0F, 0.01F));
	}

	SECTION("rawrbox::getPoint::tangent::normal") {
		rawrbox::Vector3f tangent = {};
		rawrbox::Vector3f normal = {};

		curve.getPoint(0.5F, tangent, normal);
		REQUIRE_THAT(tangent.dot(normal), Catch::Matchers::WithinAbs(0.0F, 0.01F));
	}

	SECTION("rawrbox::getOrientedPoint") {
		auto orientedPoint = curve.getOrientedPoint(0.5F);

		REQUIRE_THAT(orientedPoint.position.x, Catch::Matchers::WithinAbs(1.5F, 0.01F));
		REQUIRE_THAT(orientedPoint.position.y, Catch::Matchers::WithinAbs(2.0F, 0.01F));
		REQUIRE_THAT(orientedPoint.position.z, Catch::Matchers::WithinAbs(0.0F, 0.01F));
		REQUIRE_THAT(orientedPoint.rotation.length(), Catch::Matchers::WithinAbs(1.0F, 0.01F));
	}
}
