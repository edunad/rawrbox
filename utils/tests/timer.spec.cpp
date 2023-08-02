#include <rawrbox/utils/timer.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Timer should behave as expected", "[rawrbox::Timer]") {
	SECTION("rawrbox::Timer::simple") {
		REQUIRE(rawrbox::Timer::timers.empty() == true);

		rawrbox::Timer::simple(0, []() {});
		REQUIRE(rawrbox::Timer::timers.size() == 1);

		rawrbox::Timer::clear();
		REQUIRE(rawrbox::Timer::timers.empty() == true);
	}

	SECTION("rawrbox::Timer::create") {
		REQUIRE(rawrbox::Timer::timers.empty() == true);

		rawrbox::Timer::create(2, 0, []() {});
		REQUIRE(rawrbox::Timer::timers.size() == 1);

		rawrbox::Timer::clear();
		REQUIRE(rawrbox::Timer::timers.empty() == true);
	}
}
