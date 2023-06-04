
#include <rawrbox/engine/engine.hpp>
#include <rawrbox/engine/static.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <thread>

TEST_CASE("Engine should behave as expected", "[rawrbox::Engine]") {
	rawrbox::Engine eng;

	SECTION("rawrbox::Engine::setTPS") {
		REQUIRE(eng.getTPS() == 66);
		eng.setTPS(10);
		REQUIRE(eng.getTPS() == 10);
	}

	SECTION("rawrbox::Engine::setFPS") {
		REQUIRE(eng.getFPS() == 60);
		eng.setFPS(10);
		REQUIRE(eng.getFPS() == 10);
	}

	SECTION("rawrbox::Engine::shutdown") {
		REQUIRE(eng.isQuitting() == false);
		eng.shutdown();
		REQUIRE(eng.isQuitting() == true);
		REQUIRE_NOTHROW(eng.run());
	}

	SECTION("rawrbox::Engine::runOnMainThread") {
		REQUIRE(rawrbox::MAIN_THREAD_INVOKES.isEmpty() == true);
		std::thread t1([](const std::string& msg) {
			REQUIRE(msg == "nice");
			rawrbox::runOnMainThread([]() {
				REQUIRE(true);
			});
		},
		    "nice");

		rawrbox::___runThreadInvokes();
		t1.join();
		REQUIRE(rawrbox::MAIN_THREAD_INVOKES.isEmpty() == false);
	}
}
