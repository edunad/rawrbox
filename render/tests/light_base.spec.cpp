
#include <rawrbox/render/light/point.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>

TEST_CASE("Lights should behave as expected", "[rawrbox::LightBase]") {
	auto base = std::make_shared<rawrbox::PointLight>(rawrbox::Vector3f{5, -10, 0}, rawrbox::Colors::Red(), 0.F);

	SECTION("rawrbox::LightBase::isOn") {
		REQUIRE(base->isOn() == true);
		base->setStatus(false);
		REQUIRE(base->isOn() == false);
	}

	SECTION("rawrbox::LightBase::getWorldPos") {
		auto pos = base->getWorldPos();

		REQUIRE(pos.x == 5);
		REQUIRE(pos.y == -10);
		REQUIRE(pos.z == 0);
	}

	SECTION("rawrbox::LightBase::getType") {
		REQUIRE(base->getType() == rawrbox::LightType::POINT);
	}
}
