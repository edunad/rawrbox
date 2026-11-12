#include <rawrbox/utils/event.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace rawrbox {
	static int eventFirstCalls = 0;
	static int eventSecondCalls = 0;

	static void onEventFirst(int /*_v*/) { ++eventFirstCalls; }
	static void onEventSecond(int /*_v*/) { ++eventSecondCalls; }
	static void onEventFirstRef(const std::string& /*_v*/) { ++eventFirstCalls; }
	static void onEventSecondRef(const std::string& /*_v*/) { ++eventSecondCalls; }
} // namespace rawrbox

TEST_CASE("Event should behave as expected", "[rawrbox::Event]") {
	SECTION("rawrbox::Event::size") {
		rawrbox::Event<std::string> a;
		REQUIRE(a.empty() == true);
		REQUIRE(a.size() == 0);

		a += [](const std::string& /*_t*/) {};
		REQUIRE(a.size() == 1);

		a.clear();
		REQUIRE(a.size() == 0);
	}

	SECTION("rawrbox::Event::call") {
		rawrbox::Event<std::string> a;
		a += [](const std::string& t) {
			REQUIRE(t == "ok");
		};

		a("ok");
	}

	SECTION("rawrbox::Event::remove function pointer") {
		rawrbox::eventFirstCalls = 0;
		rawrbox::eventSecondCalls = 0;

		rawrbox::Event<int> a;
		a += rawrbox::onEventFirst;
		a += rawrbox::onEventSecond;
		REQUIRE(a.size() == 2);

		a -= rawrbox::onEventFirst;
		REQUIRE(a.size() == 1);

		a(1);
		REQUIRE(rawrbox::eventFirstCalls == 0);
		REQUIRE(rawrbox::eventSecondCalls == 1);

		a -= rawrbox::onEventSecond;
		REQUIRE(a.empty() == true);
	}

	SECTION("rawrbox::Event::remove function pointer with converted signature") {
		rawrbox::eventFirstCalls = 0;
		rawrbox::eventSecondCalls = 0;

		rawrbox::Event<std::string> a;
		a += rawrbox::onEventFirstRef;
		a += rawrbox::onEventSecondRef;

		a -= rawrbox::onEventSecondRef;
		REQUIRE(a.size() == 1);

		a("ok");
		REQUIRE(rawrbox::eventFirstCalls == 1);
		REQUIRE(rawrbox::eventSecondCalls == 0);
	}

	SECTION("rawrbox::Event::remove std::function holding a pointer") {
		rawrbox::eventFirstCalls = 0;
		rawrbox::eventSecondCalls = 0;

		rawrbox::Event<int> a;
		a += rawrbox::onEventFirst;
		a += rawrbox::onEventSecond;

		rawrbox::Event<int>::Func func = rawrbox::onEventFirst;
		a -= func;
		REQUIRE(a.size() == 1);

		a(1);
		REQUIRE(rawrbox::eventFirstCalls == 0);
		REQUIRE(rawrbox::eventSecondCalls == 1);
	}

	SECTION("rawrbox::Event::remove lambda") {
		int calls = 0;
		auto lambda = [&calls](int /*_v*/) { ++calls; };

		rawrbox::Event<int> a;
		a += lambda;
		a += [&calls](int /*_v*/) { calls += 10; };
		a += rawrbox::onEventFirst;

		a -= lambda;
		REQUIRE(a.size() == 2);

		a(1);
		REQUIRE(calls == 10);
	}

	SECTION("rawrbox::Event::+= / -= ids") {
		std::vector<int> calls = {};
		const auto make = [&calls](int value) { return [&calls, value](int /*_v*/) { calls.push_back(value); }; };

		rawrbox::Event<int> a;
		const size_t one = a += make(1);
		const size_t two = a += make(2);
		const size_t three = a += make(3);

		REQUIRE(one != 0);
		REQUIRE(one != two);
		REQUIRE(a.size() == 3);

		a -= two;
		REQUIRE(a.size() == 2);

		a -= two;
		a -= 0;
		REQUIRE(a.size() == 2);

		a(0);
		REQUIRE(calls == std::vector<int>{1, 3});

		a -= one;
		a -= three;
		REQUIRE(a.empty());
	}

	SECTION("rawrbox::Event::-= while firing") {
		std::vector<int> calls = {};

		rawrbox::Event<int> a;
		size_t second = 0;

		const size_t first = a += [&](int /*_v*/) {
			calls.push_back(1);
			a -= second;
		};
		second = a += [&](int /*_v*/) { calls.push_back(2); };

		size_t self = 0;
		self = a += [&](int /*_v*/) {
			calls.push_back(3);
			a -= self;
		};

		a(0);
		REQUIRE(calls == std::vector<int>{1, 3});
		REQUIRE(a.size() == 1);

		calls.clear();
		a(0);
		REQUIRE(calls == std::vector<int>{1});
		a -= first;
		REQUIRE(a.empty());
	}

	SECTION("rawrbox::Event::+= while firing") {
		int calls = 0;
		rawrbox::Event<int> a;

		a += [&](int /*_v*/) {
			++calls;
			if (calls == 1) a += ([&](int /*_v*/) { calls += 10; });
		};

		a(0);
		REQUIRE(calls == 1);
		REQUIRE(a.size() == 2);

		a(0);
		REQUIRE(calls == 12);
	}

	SECTION("rawrbox::Event::nested fire and clear") {
		int calls = 0;
		rawrbox::Event<int> a;

		a += [&](int depth) {
			++calls;
			if (depth == 0) a(1);
			if (depth == 1) a.clear();
		};
		a += [&](int /*_v*/) { calls += 100; };

		a(0);
		REQUIRE(calls == 2);
		REQUIRE(a.empty());

		a(0);
		REQUIRE(calls == 2);
	}

	SECTION("rawrbox::Event::==") {
		rawrbox::Event<int> a;
		rawrbox::Event<int> b;

		a += rawrbox::onEventFirst;
		b += rawrbox::onEventFirst;

		REQUIRE(a == a);
		REQUIRE_FALSE(a != a);

		REQUIRE(a != b);
		REQUIRE_FALSE(a == b);
	}
}