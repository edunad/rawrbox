#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <memory>
#include <string>

TEST_CASE("ModelBase should behave as expected", "[rawrbox::ModelBase]") {

	SECTION("rawrbox::Model::addMesh / rawrbox::Model::removeMesh") {
		rawrbox::Model<> mdl;

		mdl.addMesh(rawrbox::MeshUtils::generatePlane({}, {1, 1}));
		REQUIRE(mdl.getMesh(0) != nullptr);

		mdl.removeMesh(1);
		REQUIRE(mdl.getMesh(0) != nullptr);

		mdl.removeMesh(0);
		REQUIRE(mdl.getMesh(0) == nullptr);
	}

	SECTION("rawrbox::Model::getMeshByName") {
		rawrbox::Model<> mdl;

		REQUIRE(mdl.getMesh(0) == nullptr);

		auto m = rawrbox::MeshUtils::generatePlane({}, {1, 1});
		m.setName("test");
		mdl.addMesh(m);

		REQUIRE(mdl.getMesh(0) != nullptr);
		REQUIRE(mdl.getMeshByName("aaa") == nullptr);
		REQUIRE(mdl.getMeshByName("test") != nullptr);
	}

	SECTION("rawrbox::ModelBase::merge") {
		rawrbox::ModelBase<> base;

		auto m1 = rawrbox::MeshUtils::generatePlane({}, {1, 1});
		auto m2 = rawrbox::MeshUtils::generatePlane({}, {1, 1});

		REQUIRE(m1.baseIndex == 0);
		REQUIRE(m1.baseVertex == 0);
		REQUIRE(m1.totalIndex == 6);
		REQUIRE(m1.totalVertex == 4);

		m1.merge(m2);

		REQUIRE(m1.baseIndex == 0);
		REQUIRE(m1.baseVertex == 0);
		REQUIRE(m1.totalIndex == 12);
		REQUIRE(m1.totalVertex == 8);
	}

	SECTION("rawrbox::Model::getBBOX") {
		rawrbox::Model<> mdl;

		auto& b = mdl.getBBOX();
		REQUIRE(b.isEmpty() == true);
		REQUIRE(b.size() == 0.F);

		mdl.addMesh(rawrbox::MeshUtils::generatePlane({}, {2, 2}));

		REQUIRE(b.isEmpty() == false);
		REQUIRE(b.size().x == 2.F);
		REQUIRE(b.size().y == 2.F);
		REQUIRE(b.size().z == 0.F);

		mdl.addMesh(rawrbox::MeshUtils::generateCube({}, {2, 4, 2}));

		REQUIRE(b.size().x == 2.F);
		REQUIRE(b.size().y == 4.F);
		REQUIRE(b.size().z == 2.F);
	}

	SECTION("rawrbox::ModelBase::setPos / rawrbox::ModelBase::getPos") {
		rawrbox::ModelBase<> base;

		REQUIRE(base.getPos().x == 0);
		REQUIRE(base.getPos().y == 0);
		REQUIRE(base.getPos().z == 0);

		base.setPos({12, 3, 8});

		REQUIRE(base.getPos().x == 12);
		REQUIRE(base.getPos().y == 3);
		REQUIRE(base.getPos().z == 8);
	}

	SECTION("rawrbox::ModelBase::setAngle / rawrbox::ModelBase::getAngle") {
		rawrbox::ModelBase<> base;
		REQUIRE(base.getAngle().x == 0);
		REQUIRE(base.getAngle().y == 0);
		REQUIRE(base.getAngle().z == 0);
		REQUIRE(base.getAngle().w == 0);

		base.setAngle({12, 3, 8, 32});

		REQUIRE(base.getAngle().x == 12);
		REQUIRE(base.getAngle().y == 3);
		REQUIRE(base.getAngle().z == 8);
		REQUIRE(base.getAngle().w == 32);
	}

	SECTION("rawrbox::ModelBase::setEulerAngle / rawrbox::ModelBase::getAngle") {
		rawrbox::ModelBase<> base;
		REQUIRE(base.getAngle().x == 0);
		REQUIRE(base.getAngle().y == 0);
		REQUIRE(base.getAngle().z == 0);
		REQUIRE(base.getAngle().w == 0);

		base.setEulerAngle({12, 3, 8});

		REQUIRE_THAT(base.getAngle().x, Catch::Matchers::WithinAbs(0.73776F, 0.0001F));
		REQUIRE_THAT(base.getAngle().y, Catch::Matchers::WithinAbs(0.61108F, 0.0001F));
		REQUIRE_THAT(base.getAngle().z, Catch::Matchers::WithinAbs(0.13078F, 0.0001F));
		REQUIRE_THAT(base.getAngle().w, Catch::Matchers::WithinAbs(-0.25533F, 0.0001F));
	}

	SECTION("rawrbox::ModelBase::setScale / rawrbox::ModelBase::getScale") {
		rawrbox::ModelBase<> base;
		REQUIRE(base.getScale().x == 1);
		REQUIRE(base.getScale().y == 1);
		REQUIRE(base.getScale().z == 1);

		base.setScale({12, 3, 8});

		REQUIRE(base.getScale().x == 12);
		REQUIRE(base.getScale().y == 3);
		REQUIRE(base.getScale().z == 8);
	}

	SECTION("rawrbox::Model::setCulling") {
		rawrbox::Model<> mdl;
		mdl.addMesh(rawrbox::MeshUtils::generatePlane({}, {1, 1}));
		mdl.addMesh(rawrbox::MeshUtils::generatePlane({}, {1, 1}));

		REQUIRE(mdl.getMesh(0)->culling == BGFX_STATE_CULL_CW);
		REQUIRE(mdl.getMesh(1)->culling == BGFX_STATE_CULL_CW);
		mdl.setCulling(BGFX_STATE_CULL_CCW, 0);
		REQUIRE(mdl.getMesh(0)->culling == BGFX_STATE_CULL_CCW);
		REQUIRE(mdl.getMesh(1)->culling == BGFX_STATE_CULL_CW);
		mdl.setCulling(BGFX_STATE_CULL_CCW);
		REQUIRE(mdl.getMesh(0)->culling == BGFX_STATE_CULL_CCW);
		REQUIRE(mdl.getMesh(1)->culling == BGFX_STATE_CULL_CCW);
	}

	SECTION("rawrbox::Model::setWireframe") {
		rawrbox::Model<> mdl;
		mdl.addMesh(rawrbox::MeshUtils::generatePlane({}, {1, 1}));
		mdl.addMesh(rawrbox::MeshUtils::generatePlane({}, {1, 1}));

		REQUIRE(mdl.getMesh(0)->wireframe == false);
		REQUIRE(mdl.getMesh(1)->wireframe == false);
		mdl.setWireframe(true, 0);
		REQUIRE(mdl.getMesh(0)->wireframe == true);
		REQUIRE(mdl.getMesh(1)->wireframe == false);
		mdl.setWireframe(true);
		REQUIRE(mdl.getMesh(0)->wireframe == true);
		REQUIRE(mdl.getMesh(1)->wireframe == true);
	}

	SECTION("rawrbox::Model::setBlend") {
		rawrbox::Model<> mdl;
		mdl.addMesh(rawrbox::MeshUtils::generatePlane({}, {1, 1}));
		mdl.addMesh(rawrbox::MeshUtils::generatePlane({}, {1, 1}));

		REQUIRE(mdl.getMesh(0)->blending == BGFX_STATE_BLEND_NORMAL);
		REQUIRE(mdl.getMesh(1)->blending == BGFX_STATE_BLEND_NORMAL);
		mdl.setBlend(BGFX_STATE_BLEND_SRC_ALPHA, 0);

		REQUIRE(mdl.getMesh(0)->blending == BGFX_STATE_BLEND_SRC_ALPHA);
		REQUIRE(mdl.getMesh(1)->blending == BGFX_STATE_BLEND_NORMAL);
		mdl.setBlend(BGFX_STATE_BLEND_SRC_ALPHA);

		REQUIRE(mdl.getMesh(0)->blending == BGFX_STATE_BLEND_SRC_ALPHA);
		REQUIRE(mdl.getMesh(1)->blending == BGFX_STATE_BLEND_SRC_ALPHA);
	}
}
