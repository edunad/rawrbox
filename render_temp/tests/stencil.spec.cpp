
#include <rawrbox/render_temp/stencil.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Stencil should behave as expected", "[rawrbox::Stencil]") {
	rawrbox::Stencil base({100, 100});

	SECTION("rawrbox::Stencil::drawTriangle") {
		base.clear();

		REQUIRE(base.getDrawCalls().size() == 0);
		base.drawTriangle({0, 0}, {0, 0}, rawrbox::Colors::Black(), {0, 10}, {0, 1}, rawrbox::Colors::Black(), {10, 0}, {1, 0}, rawrbox::Colors::Black());
		REQUIRE(base.getDrawCalls().size() == 1);
		REQUIRE(base.getDrawCalls().front().vertices.size() == 3);
	}

	SECTION("rawrbox::Stencil::drawCircle") {
		base.clear();

		REQUIRE(base.getDrawCalls().size() == 0);
		base.drawCircle({0, 0}, {10, 10}, rawrbox::Colors::Black());
		REQUIRE(base.getDrawCalls().size() == 1);
		REQUIRE(base.getDrawCalls().front().vertices.size() == 96);
	}

	SECTION("rawrbox::Stencil::drawLine") {
		base.clear();

		REQUIRE(base.getDrawCalls().size() == 0);
		base.drawLine({0, 0}, {10, 10}, rawrbox::Colors::Black());
		REQUIRE(base.getDrawCalls().size() == 1);
		REQUIRE(base.getDrawCalls().front().vertices.size() == 2);
	}

	SECTION("rawrbox::Stencil::drawBox") {
		base.clear();

		REQUIRE(base.getDrawCalls().size() == 0);

		base.pushOutline({1.F}); // Draw box uses texture, so we need to draw outline instead, since bgfx is not active
		base.drawBox({0, 0}, {10, 10}, rawrbox::Colors::Black());
		base.popOutline();

		REQUIRE(base.getDrawCalls().size() == 1);
		REQUIRE(base.getDrawCalls().front().vertices.size() == 8);
	}
}
