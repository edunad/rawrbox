#include <rawrbox/utils/timer.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("TIMER should behave as expected", "[rawrbox::TIMER]") {
	SECTION("rawrbox::TIMER::simple") {
		REQUIRE(rawrbox::TIMER::timers.empty() == true);

		rawrbox::TIMER::simple(0, []() {});
		REQUIRE(rawrbox::TIMER::timers.size() == 1);

		rawrbox::TIMER::clear();
		REQUIRE(rawrbox::TIMER::timers.empty() == true);
	}

	SECTION("rawrbox::Timer::create") {
		REQUIRE(rawrbox::TIMER::timers.empty() == true);

		rawrbox::TIMER::create(2, 0, []() {});
		REQUIRE(rawrbox::TIMER::timers.size() == 1);

		rawrbox::TIMER::clear();
		REQUIRE(rawrbox::TIMER::timers.empty() == true);
	}
}
