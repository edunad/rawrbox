#include <rawrbox/utils/tasker.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Tasker should behave as expected", "[rawrbox::Tasker]") {

	struct TestTask : public rawrbox::Task {
		std::function<void()> callback = nullptr;

		void run(const std::function<void()>& onComplete) override {
			if (callback != nullptr) callback();
			onComplete();
		}
	};

	SECTION("rawrbox::Tasker::run") {
		rawrbox::Tasker<TestTask> tasker(3);

		TestTask task = {};
		task.callback = []() {
			REQUIRE(true);
		};

		REQUIRE_NOTHROW(tasker.addTask(task));
		REQUIRE(tasker.total() == 0);

		for (size_t i = 0; i < 10; i++) {
			REQUIRE_NOTHROW(tasker.addTask(task));
		}

		REQUIRE(tasker.total() == 0);
	}
}
