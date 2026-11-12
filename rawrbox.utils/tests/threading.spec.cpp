#include <rawrbox/utils/threading.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <memory>
#include <stdexcept>

TEST_CASE("ASYNC should behave as expected", "[rawrbox::ASYNC]") {
	rawrbox::ASYNC::init(2);

	auto value = rawrbox::ASYNC::submit([]() { return 42; });
	REQUIRE(value.get() == 42);

	std::atomic<int> runs = 0;
	auto job = rawrbox::ASYNC::submit([&runs]() { runs++; });
	job.get();
	REQUIRE(runs == 1);

	auto moveOnly = rawrbox::ASYNC::submit([ptr = std::make_unique<int>(7)]() { return *ptr; });
	REQUIRE(moveOnly.get() == 7);

	auto failed = rawrbox::ASYNC::submit([]() -> int { throw std::runtime_error("failed"); });
	REQUIRE_THROWS_AS(failed.get(), std::runtime_error);

	rawrbox::ASYNC::shutdown();

	auto closed = rawrbox::ASYNC::submit([]() { return 1; });
	REQUIRE_THROWS_AS(closed.get(), std::runtime_error);
}
