#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/mesh.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>

TEST_CASE("Mesh should behave as expected", "[rawrBox::Mesh]") {
	std::shared_ptr<rawrBox::Mesh<rawrBox::VertexData>> base = std::make_shared<rawrBox::Mesh<rawrBox::VertexData>>();

	SECTION("rawrBox::Mesh::setName / rawrBox::Mesh::getName") {
		REQUIRE(base->getName() == "mesh");
		base->setName("test");
		REQUIRE(base->getName() == "test");
	}

	SECTION("rawrBox::Mesh::setWireframe") {
		base->setWireframe(true);
		REQUIRE(base->wireframe == true);
	}

	SECTION("rawrBox::Mesh::setCulling") {
		REQUIRE(base->culling == BGFX_STATE_CULL_CW);
		base->setCulling(BGFX_STATE_CULL_CCW);
		REQUIRE(base->culling == BGFX_STATE_CULL_CCW);
	}

	SECTION("rawrBox::Mesh::setBlend") {
		REQUIRE(base->blending == BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
		base->setBlend(BGFX_STATE_BLEND_ADD);
		REQUIRE(base->blending == BGFX_STATE_BLEND_ADD);
	}

	SECTION("rawrBox::Mesh::setColor") {
		REQUIRE(base->color == rawrBox::Colors::White);
		base->setColor(rawrBox::Colors::Orange);
		REQUIRE(base->color == rawrBox::Colors::Orange);
	}

	SECTION("rawrBox::Mesh::getBBOX") {
		auto& b = base->getBBOX();

		REQUIRE(b.isEmpty() == true);
		REQUIRE(b.size() == 0.F);
	}

	SECTION("rawrBox::Mesh::addData / rawrBox::Mesh::getData / rawrBox::Mesh::hasData") {
		base->data.clear();
		REQUIRE(base->data.size() == 0);

		REQUIRE(base->hasData("test") == false);
		REQUIRE_THROWS(base->getData("test"));
		REQUIRE_NOTHROW(base->addData("test", {0, 0, 0}));
		REQUIRE(base->hasData("test") == true);

		REQUIRE_THROWS(base->addData("test", {0, 0, 0}));
		REQUIRE_NOTHROW(base->getData("test"));
	}

	SECTION("rawrBox::Mesh::canMerge") {
		std::shared_ptr<rawrBox::Mesh<>> base2 = std::make_shared<rawrBox::Mesh<>>();
		std::shared_ptr<rawrBox::Mesh<>> base3 = std::make_shared<rawrBox::Mesh<>>();

		REQUIRE(base2->canMerge(base3) == true);

		base2->setWireframe(true);
		REQUIRE(base2->canMerge(base3) == false);
	}
}
