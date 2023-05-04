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
	}
}
