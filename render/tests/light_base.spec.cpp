
#include <rawrbox/render/model/light/directional.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>

TEST_CASE("Lights should behave as expected", "[rawrBox::LightBase]") {
	/*auto base = std::make_shared<rawrBox::LightDirectional>(rawrBox::Vector3f{5, -10, 0}, rawrBox::Vector3f{0, 9.F, 0}, rawrBox::Colors::Red, rawrBox::Colors::Gray);

	SECTION("rawrBox::LightBase::isOn") {
		REQUIRE(base->isOn() == true);
		base->setStatus(false);
		REQUIRE(base->isOn() == false);
	}

	SECTION("rawrBox::LightBase::getPosMatrix") {
		auto pos = base->getPosMatrix();

		REQUIRE(pos[0] == 5);
		REQUIRE(pos[1] == -10);
		REQUIRE(pos[2] == 0);
		REQUIRE(pos[3] == 0);
	}

	SECTION("rawrBox::LightBase::getType") {
		REQUIRE(base->getType() == rawrBox::LightType::LIGHT_DIR);
	}

	SECTION("rawrBox::LightBase::getDataMatrix") {
		auto data = base->getDataMatrix();

		REQUIRE(data[0] == 1);
		REQUIRE(data[4] == 0.32F);
		REQUIRE(data[8] == 0.32F);
		REQUIRE(data[12] == 0);

		REQUIRE(data[1] == 0.81F);
		REQUIRE(data[5] == 0.8F);
		REQUIRE(data[9] == 0.75F);
		REQUIRE(data[13] == 0);

		REQUIRE(data[2] == 0.F);
		REQUIRE(data[6] == 9.0F);
		REQUIRE(data[10] == 0.F);

		REQUIRE(data[15] == static_cast<float>(base->isOn()));

		REQUIRE(data[3] == static_cast<float>(base->getType()));
		REQUIRE(data[7] == 0.F);
		REQUIRE(data[11] == 0.F);
	}*/
}
