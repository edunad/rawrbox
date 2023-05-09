
#include <rawrbox/render/stencil.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <bx/math.h>

#include <string>

TEST_CASE("Stencil should behave as expected", "[rawrbox::Stencil]") {
	rawrbox::Stencil base({100, 100});

	SECTION("rawrbox::Stencil::drawTriangle") {
		base.clear();

		REQUIRE(base.getVertices().size() == 0);
		base.drawTriangle({0, 0}, {0, 0}, rawrbox::Colors::Black, {0, 10}, {0, 1}, rawrbox::Colors::Black, {10, 0}, {1, 0}, rawrbox::Colors::Black);
		REQUIRE(base.getVertices().size() == 3);
	}

	SECTION("rawrbox::Stencil::drawCircle") {
		base.clear();

		REQUIRE(base.getVertices().size() == 0);
		base.drawCircle({0, 0}, {10, 10}, rawrbox::Colors::Black);
		REQUIRE(base.getVertices().size() == 96);
	}

	SECTION("rawrbox::Stencil::drawLine") {
		base.clear();

		REQUIRE(base.getVertices().size() == 0);
		base.drawLine({0, 0}, {10, 10}, rawrbox::Colors::Black);
		REQUIRE(base.getVertices().size() == 2);
	}

	SECTION("rawrbox::Stencil::drawBox") {
		base.clear();

		REQUIRE(base.getVertices().size() == 0);
		base.drawBox({0, 0}, {10, 10}, rawrbox::Colors::Black);
		REQUIRE(base.getVertices().size() == 4);
	}
}
