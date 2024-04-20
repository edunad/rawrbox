
#include <rawrbox/engine/engine.hpp>
#include <rawrbox/engine/static.hpp>

#include <catch2/catch_test_macros.hpp>

#include <thread>

std::atomic<bool> shutdownThread = false;
std::atomic<int> threadCalls = 0;

TEST_CASE("Engine should behave as expected", "[rawrbox::Engine]") {
	rawrbox::Engine eng;

	auto curtime = []() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); };

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
	}

	SECTION("rawrbox::Engine::runOnRenderThread") {
		REQUIRE(rawrbox::RENDER_THREAD_INVOKES.empty() == true);

		long long delay = 0;
		threadCalls = 0;

		rawrbox::RENDER_THREAD_ID = std::this_thread::get_id();
		rawrbox::runOnRenderThread([]() {
			auto t1 = std::jthread([]() {
				threadCalls++;
				rawrbox::runOnRenderThread([]() {
					threadCalls++;
					auto t2 = std::jthread([]() {
						rawrbox::runOnRenderThread([]() {
							threadCalls++;
							auto t3 = std::jthread([]() {
								rawrbox::runOnRenderThread([]() {
									threadCalls++;
									shutdownThread = true;
								});
							});
						});
					});
				});
			});
		});

		delay = curtime() + 1500;
		while (!shutdownThread && delay > curtime()) {
			rawrbox::___runThreadInvokes();
		}

		REQUIRE(rawrbox::RENDER_THREAD_INVOKES.empty());
		REQUIRE(threadCalls == 4);
	}
}
