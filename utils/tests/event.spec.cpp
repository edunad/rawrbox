#include <rawrbox/utils/event.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Event should behave as expected", "[rawrbox::Event]") {
	SECTION("rawrbox::Event::size") {
		rawrbox::Event<std::string> a;
		REQUIRE(a.empty() == true);
		REQUIRE(a.size() == 0);

		a += [](std::string /*_t*/) {};
		REQUIRE(a.size() == 1);

		a.clear();
		REQUIRE(a.size() == 0);
	}

	SECTION("rawrbox::Event::call") {
		rawrbox::Event<std::string> a;
		a += [](std::string t) {
			REQUIRE(t == "ok");
		};

		a("ok");
	}
}
