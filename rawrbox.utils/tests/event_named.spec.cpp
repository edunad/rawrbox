#include <rawrbox/utils/event_named.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Event should behave as expected", "[rawrbox::EventNamed]") {
	SECTION("rawrbox::EventNamed::size") {
		rawrbox::EventNamed<std::string> a;
		REQUIRE(a.size() == 0);

		a.add("test", [](const std::string& /*_t*/) {});
		a.add("test2", [](const std::string& /*_t*/) {});
		REQUIRE(a.size() == 2);

		a.remove("test2");
		REQUIRE(a.size() == 1);

		a.clear();
		REQUIRE(a.size() == 0);
	}

	SECTION("rawrbox::EventNamed::call") {
		rawrbox::EventNamed<std::string> a;

		a.add("test", [](const std::string& t) { REQUIRE(t == "ok"); });
		a("ok");
	}
}
