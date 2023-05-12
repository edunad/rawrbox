#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/mesh.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>

TEST_CASE("Mesh should behave as expected", "[rawrbox::Mesh]") {
	std::shared_ptr<rawrbox::Mesh<rawrbox::VertexData>> base = std::make_shared<rawrbox::Mesh<rawrbox::VertexData>>();

	SECTION("rawrbox::Mesh::setName / rawrbox::Mesh::getName") {
		REQUIRE(base->getName() == "mesh");
		base->setName("test");
		REQUIRE(base->getName() == "test");
	}

	SECTION("rawrbox::Mesh::setWireframe") {
		base->setWireframe(true);
		REQUIRE(base->wireframe == true);
	}

	SECTION("rawrbox::Mesh::setCulling") {
		REQUIRE(base->culling == BGFX_STATE_CULL_CW);
		base->setCulling(BGFX_STATE_CULL_CCW);
		REQUIRE(base->culling == BGFX_STATE_CULL_CCW);
	}

	SECTION("rawrbox::Mesh::setBlend") {
		REQUIRE(base->blending == BGFX_STATE_BLEND_ALPHA_TO_COVERAGE);
		base->setBlend(BGFX_STATE_BLEND_ADD);
		REQUIRE(base->blending == BGFX_STATE_BLEND_ADD);
	}

	SECTION("rawrbox::Mesh::setColor") {
		REQUIRE(base->color == rawrbox::Colors::White);
		base->setColor(rawrbox::Colors::Orange);
		REQUIRE(base->color == rawrbox::Colors::Orange);
	}

	SECTION("rawrbox::Mesh::getBBOX") {
		auto& b = base->getBBOX();

		REQUIRE(b.isEmpty() == true);
		REQUIRE(b.size() == 0.F);
	}

	SECTION("rawrbox::Mesh::addData / rawrbox::Mesh::getData / rawrbox::Mesh::hasData") {
		base->data.clear();
		REQUIRE(base->data.size() == 0);

		REQUIRE(base->hasData("test") == false);
		REQUIRE_THROWS(base->getData("test"));
		REQUIRE_NOTHROW(base->addData("test", {0, 0, 0}));
		REQUIRE(base->hasData("test") == true);

		REQUIRE_NOTHROW(base->getData("test"));
	}

	SECTION("rawrbox::Mesh::canOptimize") {
		std::shared_ptr<rawrbox::Mesh<>> base2 = std::make_shared<rawrbox::Mesh<>>();
		std::shared_ptr<rawrbox::Mesh<>> base3 = std::make_shared<rawrbox::Mesh<>>();

		REQUIRE(base2->canOptimize(base3) == true);

		base2->setWireframe(true);
		REQUIRE(base2->canOptimize(base3) == false);
	}
}
