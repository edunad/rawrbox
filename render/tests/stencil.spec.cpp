
#include <rawrbox/render/stencil.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <bx/math.h>

#include <string>

TEST_CASE("Stencil should behave as expected", "[rawrBox::Stencil]") {
	rawrBox::Stencil base(0, {100, 100});

	SECTION("rawrBox::Stencil::drawTriangle") {
		base.clear();

		REQUIRE(base.getVertices().size() == 0);
		base.drawTriangle({0, 0}, {0, 0}, rawrBox::Colors::Black, {0, 10}, {0, 1}, rawrBox::Colors::Black, {10, 0}, {1, 0}, rawrBox::Colors::Black);
		REQUIRE(base.getVertices().size() == 3);
	}

	SECTION("rawrBox::Stencil::drawCircle") {
		base.clear();

		REQUIRE(base.getVertices().size() == 0);
		base.drawCircle({0, 0}, {10, 10}, rawrBox::Colors::Black);
		REQUIRE(base.getVertices().size() == 96);
	}

	SECTION("rawrBox::Stencil::drawLine") {
		base.clear();

		REQUIRE(base.getVertices().size() == 0);
		base.drawLine({0, 0}, {10, 10}, rawrBox::Colors::Black);
		REQUIRE(base.getVertices().size() == 2);
	}

	SECTION("rawrBox::Stencil::drawBox") {
		base.clear();

		REQUIRE(base.getVertices().size() == 0);
		base.drawBox({0, 0}, {10, 10}, rawrBox::Colors::Black);
		REQUIRE(base.getVertices().size() == 4);
	}
}
