#include <rawrbox/render/renderer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <bx/math.h>

TEST_CASE("Renderer should behave as expected", "[rawrbox::Renderer]") {
	rawrbox::Renderer render = rawrbox::Renderer(0, {100, 100});

	SECTION("rawrbox::Renderer::setClearColor") {
		render.setClearColor(0x00FF00FF);
		REQUIRE(render.getClearColor() == 0x00FF00FF);
	}

	SECTION("rawrbox::Renderer::getID") {
		REQUIRE(render.getID() == 0);
	}

	SECTION("rawrbox::Renderer::getSize") {
		REQUIRE(render.getSize().x == 100);
		REQUIRE(render.getSize().y == 100);
	}
}
