#include <rawrbox/render/window.hpp>

#include <catch2/catch_test_macros.hpp>

#include <thread>

TEST_CASE("WindowInputState should behave as expected", "[rawrbox::WindowInputState]") {
	rawrbox::WindowInputState<rawrbox::KEY_LAST + 1> keys = {};

	SECTION("rawrbox::WindowInputState::isDown") {
		keys.press(rawrbox::KEY_A);
		keys.press(rawrbox::KEY_MENU);
		REQUIRE(keys.isDown(rawrbox::KEY_A));
		REQUIRE(keys.isDown(rawrbox::KEY_MENU));
		REQUIRE_FALSE(keys.isDown(rawrbox::KEY_B));

		keys.release(rawrbox::KEY_A);
		REQUIRE_FALSE(keys.isDown(rawrbox::KEY_A));

		REQUIRE_FALSE(keys.isDown(-1));
		REQUIRE_FALSE(keys.isDown(rawrbox::KEY_LAST + 1));
		keys.press(-1);
	}

	SECTION("rawrbox::WindowInputState::latch") {
		keys.press(rawrbox::KEY_W);
		REQUIRE_FALSE(keys.isPressed(rawrbox::KEY_W));

		keys.reset();
		REQUIRE(keys.isPressed(rawrbox::KEY_W));
		REQUIRE_FALSE(keys.isReleased(rawrbox::KEY_W));

		keys.reset();
		REQUIRE_FALSE(keys.isPressed(rawrbox::KEY_W));
		REQUIRE(keys.isDown(rawrbox::KEY_W));

		keys.release(rawrbox::KEY_W);
		keys.press(rawrbox::KEY_SPACE);
		keys.release(rawrbox::KEY_SPACE);
		keys.reset();

		REQUIRE(keys.isReleased(rawrbox::KEY_W));
		REQUIRE(keys.isPressed(rawrbox::KEY_SPACE));
		REQUIRE(keys.isReleased(rawrbox::KEY_SPACE));
		REQUIRE_FALSE(keys.isDown(rawrbox::KEY_SPACE));
	}

	SECTION("rawrbox::WindowInputState from another thread") {
		std::jthread input([&keys]() {
			for (int i = 0; i < 1000; i++) {
				keys.press(rawrbox::KEY_D);
				keys.release(rawrbox::KEY_D);
			}

			keys.press(rawrbox::KEY_D);
		});

		input.join();
		keys.reset();

		REQUIRE(keys.isDown(rawrbox::KEY_D));
		REQUIRE(keys.isPressed(rawrbox::KEY_D));
		REQUIRE(keys.isReleased(rawrbox::KEY_D));
	}
}
