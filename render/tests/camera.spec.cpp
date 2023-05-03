#include <rawrbox/render/camera/base.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <bx/math.h>

#include <string>

TEST_CASE("Camera should behave as expected", "[rawrBox::Camera]") {
	rawrBox::CameraBase base;

	SECTION("rawrBox::Camera::setPos / rawrBox::Camera::getPos") {
		base.setPos({10, 0, 5.F});

		auto p = base.getPos();
		REQUIRE(p.x == 10.F);
		REQUIRE(p.y == 0.F);
		REQUIRE(p.z == 5.F);
	}

	SECTION("rawrBox::Camera::setAngle / rawrBox::Camera::getAngle") {
		base.setAngle({0, 0, bx::toRad(90), 0});

		auto p = base.getAngle();
		REQUIRE(p.x == 0.F);
		REQUIRE_THAT(p.y, Catch::Matchers::WithinAbs(1.5708F, 0.0001F));
		REQUIRE(p.z == 0.F);
		REQUIRE(p.w == 0.F);
	}

	SECTION("rawrBox::Camera::getForward") {
		base.setAngle({0, 0, bx::toRad(90), 0});

		auto p = base.getForward();
		REQUIRE_THAT(p.x, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(p.y, Catch::Matchers::WithinAbs(1.0F, 0.0001F));
		REQUIRE_THAT(p.z, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
	}

	SECTION("rawrBox::Camera::getUp") {
		base.setAngle({0, 0, bx::toRad(90), 0});

		auto p = base.getUp();
		REQUIRE_THAT(p.x, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(p.y, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(p.z, Catch::Matchers::WithinAbs(-1.0F, 0.0001F));
	}

	SECTION("rawrBox::Camera::getRight") {
		base.setAngle({0, 0, bx::toRad(90), 0});

		auto p = base.getRight();
		REQUIRE_THAT(p.x, Catch::Matchers::WithinAbs(-1.0F, 0.0001F));
		REQUIRE_THAT(p.y, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(p.z, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
	}
}
