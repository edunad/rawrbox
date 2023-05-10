#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/camera/perspective.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <bx/math.h>

TEST_CASE("Camera should behave as expected", "[rawrbox::Camera]") {
	rawrbox::CameraBase base;
	rawrbox::CameraPerspective pers{{1080, 720}};

	SECTION("rawrbox::Camera::setPos / rawrbox::Camera::getPos") {
		base.setPos({10, 0, 5.F});

		auto p = base.getPos();
		REQUIRE(p.x == 10.F);
		REQUIRE(p.y == 0.F);
		REQUIRE(p.z == 5.F);
	}

	SECTION("rawrbox::Camera::setAngle / rawrbox::Camera::getAngle") {
		base.setAngle({0, bx::toRad(90), 0, 0});

		auto p = base.getAngle();
		REQUIRE(p.x == 0.F);
		REQUIRE_THAT(p.y, Catch::Matchers::WithinAbs(1.5708F, 0.0001F));
		REQUIRE(p.z == 0.F);
		REQUIRE(p.w == 0.F);
	}

	SECTION("rawrbox::Camera::getForward") {
		base.setAngle({0, bx::toRad(90), 0, 0});

		auto p = base.getForward();
		REQUIRE_THAT(p.x, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(p.y, Catch::Matchers::WithinAbs(1.0F, 0.0001F));
		REQUIRE_THAT(p.z, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
	}

	SECTION("rawrbox::Camera::getUp") {
		base.setAngle({0, bx::toRad(90), 0, 0});

		auto p = base.getUp();
		REQUIRE_THAT(p.x, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(p.y, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(p.z, Catch::Matchers::WithinAbs(-1.0F, 0.0001F));
	}

	SECTION("rawrbox::Camera::getRight") {
		base.setAngle({0, bx::toRad(90), 0, 0});

		auto p = base.getRight();
		REQUIRE_THAT(p.x, Catch::Matchers::WithinAbs(-1.0F, 0.0001F));
		REQUIRE_THAT(p.y, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(p.z, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
	}

	SECTION("rawrbox::Camera::worldToScreen") {
		REQUIRE_THROWS(base.worldToScreen({0, 0, 0}));
		pers.setAngle({0, bx::toRad(90), 0, 0});

		auto scr = pers.worldToScreen({0, 3, 0});
		REQUIRE_THAT(scr.x, Catch::Matchers::WithinAbs(540.0F, 0.0001F));
		REQUIRE_THAT(scr.y, Catch::Matchers::WithinAbs(360.0F, 0.0001F));
		REQUIRE_THAT(scr.z, Catch::Matchers::WithinAbs(0.01618F, 0.0001F));
	}

	SECTION("rawrbox::Camera::screenToWorld") {
		REQUIRE_THROWS(base.screenToWorld({0, 0}));
		pers.setAngle({0, 0, 0, 0});

		auto scr = pers.screenToWorld({3, 3});
		REQUIRE_THAT(scr.x, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(scr.y, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(scr.z, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
	}
}
