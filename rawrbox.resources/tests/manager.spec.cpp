#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("RESOURCES should behave as expected", "[rawrbox::RESOURCES]") {
	SECTION("rawrbox::RESOURCES::getLoaders") {
		REQUIRE(rawrbox::RESOURCES::getLoaders().size() == 1);
	}

	SECTION("rawrbox::RESOURCES::addLoader") {
		REQUIRE(rawrbox::RESOURCES::getLoaders().size() == 1);
		rawrbox::RESOURCES::addLoader<rawrbox::JSONLoader>();
		REQUIRE(rawrbox::RESOURCES::getLoaders().size() == 2);
	}

	SECTION("rawrbox::RESOURCES::preLoadFile") {
		const auto& ld = rawrbox::RESOURCES::getLoaders().back();

		REQUIRE(ld->getPreload().empty());
		rawrbox::RESOURCES::preLoadFile("./assets/json/test.json");
		REQUIRE(ld->getPreload().size() == 1);
	}
}
