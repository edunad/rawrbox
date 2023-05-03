#include <rawrbox/utils/event.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Event should behave as expected", "[rawrBox::Event]") {
	SECTION("rawrBox::Event::size") {
		rawrBox::Event<std::string> a;
		REQUIRE(a.size() == 0);

		a += [](std::string t) {};
		REQUIRE(a.size() == 1);

		a.clear();
		REQUIRE(a.size() == 0);
	}

	SECTION("rawrBox::Event::call") {
		rawrBox::Event<std::string> a;
		a += [](std::string t) {
			REQUIRE(t == "ok");
		};

		a("ok");
	}
}
