#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/base.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <memory>
#include <string>

TEST_CASE("ModelBase should behave as expected", "[rawrBox::ModelBase]") {

	SECTION("rawrBox::ModelBase::addMesh / rawrBox::ModelBase::removeMesh") {
		auto base = std::make_shared<rawrBox::ModelBase<>>();

		REQUIRE_THROWS(base->getMesh(0));
		REQUIRE_THROWS(base->getMesh(1));

		base->addMesh(base->generatePlane({}, {1, 1}));
		REQUIRE_NOTHROW(base->getMesh(0));
		REQUIRE_THROWS(base->getMesh(1));

		base->removeMesh(1);
		REQUIRE_NOTHROW(base->getMesh(0));

		base->removeMesh(0);
		REQUIRE_THROWS(base->getMesh(0));
	}

	SECTION("rawrBox::ModelBase::mergeMeshes") {
		auto base = std::make_shared<rawrBox::ModelBase<>>();

		auto m1 = base->generatePlane({}, {1, 1});
		auto m2 = base->generatePlane({}, {1, 1});

		REQUIRE(m1->baseIndex == 0);
		REQUIRE(m1->baseVertex == 0);
		REQUIRE(m1->totalIndex == 6);
		REQUIRE(m1->totalVertex == 4);

		base->mergeMeshes(m1, m2);

		REQUIRE(m1->baseIndex == 0);
		REQUIRE(m1->baseVertex == 0);
		REQUIRE(m1->totalIndex == 12);
		REQUIRE(m1->totalVertex == 8);
	}

	SECTION("rawrBox::ModelBase::setPos / rawrBox::ModelBase::getPos") {
		auto base = std::make_shared<rawrBox::ModelBase<>>();
		REQUIRE(base->getPos().x == 0);
		REQUIRE(base->getPos().y == 0);
		REQUIRE(base->getPos().z == 0);

		base->setPos({12, 3, 8});

		REQUIRE(base->getPos().x == 12);
		REQUIRE(base->getPos().y == 3);
		REQUIRE(base->getPos().z == 8);
	}

	SECTION("rawrBox::ModelBase::setAngle / rawrBox::ModelBase::getAngle") {
		auto base = std::make_shared<rawrBox::ModelBase<>>();
		REQUIRE(base->getAngle().x == 0);
		REQUIRE(base->getAngle().y == 0);
		REQUIRE(base->getAngle().z == 0);

		base->setAngle({12, 3, 8});

		REQUIRE(base->getAngle().x == 12);
		REQUIRE(base->getAngle().y == 3);
		REQUIRE(base->getAngle().z == 8);
	}

	SECTION("rawrBox::ModelBase::setScale / rawrBox::ModelBase::getScale") {
		auto base = std::make_shared<rawrBox::ModelBase<>>();
		REQUIRE(base->getScale().x == 1);
		REQUIRE(base->getScale().y == 1);
		REQUIRE(base->getScale().z == 1);

		base->setScale({12, 3, 8});

		REQUIRE(base->getScale().x == 12);
		REQUIRE(base->getScale().y == 3);
		REQUIRE(base->getScale().z == 8);
	}

	SECTION("rawrBox::ModelBase::setCulling") {
		auto base = std::make_shared<rawrBox::ModelBase<>>();
		base->addMesh(base->generatePlane({}, {1, 1}));
		base->addMesh(base->generatePlane({}, {1, 1}));

		REQUIRE(base->getMesh(0)->culling == BGFX_STATE_CULL_CW);
		REQUIRE(base->getMesh(1)->culling == BGFX_STATE_CULL_CW);
		base->setCulling(BGFX_STATE_CULL_CCW, 0);
		REQUIRE(base->getMesh(0)->culling == BGFX_STATE_CULL_CCW);
		REQUIRE(base->getMesh(1)->culling == BGFX_STATE_CULL_CW);
		base->setCulling(BGFX_STATE_CULL_CCW);
		REQUIRE(base->getMesh(0)->culling == BGFX_STATE_CULL_CCW);
		REQUIRE(base->getMesh(1)->culling == BGFX_STATE_CULL_CCW);
	}

	SECTION("rawrBox::ModelBase::setWireframe") {
		auto base = std::make_shared<rawrBox::ModelBase<>>();
		base->addMesh(base->generatePlane({}, {1, 1}));
		base->addMesh(base->generatePlane({}, {1, 1}));

		REQUIRE(base->getMesh(0)->wireframe == false);
		REQUIRE(base->getMesh(1)->wireframe == false);
		base->setWireframe(true, 0);
		REQUIRE(base->getMesh(0)->wireframe == true);
		REQUIRE(base->getMesh(1)->wireframe == false);
		base->setWireframe(true);
		REQUIRE(base->getMesh(0)->wireframe == true);
		REQUIRE(base->getMesh(1)->wireframe == true);
	}

	SECTION("rawrBox::ModelBase::setBlend") {
		auto base = std::make_shared<rawrBox::ModelBase<>>();
		base->addMesh(base->generatePlane({}, {1, 1}));
		base->addMesh(base->generatePlane({}, {1, 1}));

		REQUIRE(base->getMesh(0)->blending == BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
		REQUIRE(base->getMesh(1)->blending == BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
		base->setBlend(BGFX_STATE_BLEND_SRC_ALPHA, 0);

		REQUIRE(base->getMesh(0)->blending == BGFX_STATE_BLEND_SRC_ALPHA);
		REQUIRE(base->getMesh(1)->blending == BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
		base->setBlend(BGFX_STATE_BLEND_SRC_ALPHA);

		REQUIRE(base->getMesh(0)->blending == BGFX_STATE_BLEND_SRC_ALPHA);
		REQUIRE(base->getMesh(1)->blending == BGFX_STATE_BLEND_SRC_ALPHA);
	}
}
