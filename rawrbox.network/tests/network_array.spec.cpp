#include <rawrbox/network/network_array.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("network_array should behave as expected", "[rawrbox::network_array]") {
	SECTION("rawrbox::VectorDelta<T>") {
		rawrbox::VectorDelta<int> vDelta = {};
		REQUIRE(vDelta.changelog.empty());

		vDelta.push_back(23);
		REQUIRE(!vDelta.changelog.empty());
		REQUIRE(vDelta.changelog[0].first == true);
		REQUIRE(vDelta.changelog[0].second == 0);

		vDelta.push_back(45);
		REQUIRE(vDelta.changelog[1].first == true);
		REQUIRE(vDelta.changelog[1].second == 1);

		auto calc = vDelta.calculate();
		REQUIRE(!calc.empty());
		REQUIRE(calc[0].first == 0);
		REQUIRE(calc[0].second == 23);

		REQUIRE(calc[1].first == 1);
		REQUIRE(calc[1].second == 45);

		REQUIRE(vDelta.changelog.empty());

		vDelta.erase(vDelta.begin());
		REQUIRE(vDelta.changelog[0].first == false);
		REQUIRE(vDelta.changelog[0].second == 0);

		calc = vDelta.calculate();
		REQUIRE(!calc.empty());
		REQUIRE(calc[0].first == 0);
		REQUIRE(calc[0].second == std::nullopt);

		REQUIRE(vDelta.changelog.empty());
	}

	SECTION("rawrbox::MapDelta<T>") {
		rawrbox::MapDelta<std::string, int> vDelta = {};
		REQUIRE(vDelta.changelog.empty());

		vDelta["meow"] = 400;

		REQUIRE(!vDelta.changelog.empty());
		REQUIRE(vDelta.changelog[0].first == true);
		REQUIRE(vDelta.changelog[0].second == "meow");

		vDelta["nya"] = 34;
		REQUIRE(vDelta.changelog[1].first == true);
		REQUIRE(vDelta.changelog[1].second == "nya");

		auto calc = vDelta.calculate();
		REQUIRE(!calc.empty());
		REQUIRE(calc[0].first == "meow");
		REQUIRE(calc[0].second == 400);

		REQUIRE(calc[1].first == "nya");
		REQUIRE(calc[1].second == 34);

		REQUIRE(vDelta.changelog.empty());

		vDelta.erase("meow");
		REQUIRE(vDelta.changelog[0].first == false);
		REQUIRE(vDelta.changelog[0].second == "meow");

		calc = vDelta.calculate();
		REQUIRE(!calc.empty());
		REQUIRE(calc[0].first == "meow");
		REQUIRE(calc[0].second == std::nullopt);

		REQUIRE(vDelta.changelog.empty());
	}

	SECTION("rawrbox::UMapDelta<T>") {
		rawrbox::UMapDelta<std::string, std::string> vDelta = {};
		REQUIRE(vDelta.changelog.empty());

		vDelta["meow"] = "cat";

		REQUIRE(!vDelta.changelog.empty());
		REQUIRE(vDelta.changelog[0].first == true);
		REQUIRE(vDelta.changelog[0].second == "meow");

		vDelta["nya"] = "yes, nya";
		REQUIRE(vDelta.changelog[1].first == true);
		REQUIRE(vDelta.changelog[1].second == "nya");

		auto calc = vDelta.calculate();
		REQUIRE(!calc.empty());
		REQUIRE(calc[0].first == "meow");
		REQUIRE(calc[0].second == "cat");

		REQUIRE(calc[1].first == "nya");
		REQUIRE(calc[1].second == "yes, nya");

		REQUIRE(vDelta.changelog.empty());

		vDelta.erase("meow");
		REQUIRE(vDelta.changelog[0].first == false);
		REQUIRE(vDelta.changelog[0].second == "meow");

		calc = vDelta.calculate();
		REQUIRE(!calc.empty());
		REQUIRE(calc[0].first == "meow");
		REQUIRE(calc[0].second == std::nullopt);

		REQUIRE(vDelta.changelog.empty());
	}
}
