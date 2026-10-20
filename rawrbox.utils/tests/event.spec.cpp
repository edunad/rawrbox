#include <rawrbox/utils/event.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
	int firstCalls = 0;
	int secondCalls = 0;

	void first(int /*_v*/) { firstCalls++; }
	void second(int /*_v*/) { secondCalls++; }
	void firstRef(const std::string& /*_v*/) { firstCalls++; }
	void secondRef(const std::string& /*_v*/) { secondCalls++; }
} // namespace

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
		firstCalls = 0;
		secondCalls = 0;

		rawrbox::Event<int> a;
		a += first;
		a += second;
		REQUIRE(a.size() == 2);

		a -= first;
		REQUIRE(a.size() == 1);

		a(1);
		REQUIRE(firstCalls == 0);
		REQUIRE(secondCalls == 1);

		a -= second;
		REQUIRE(a.empty() == true);
	}

	SECTION("rawrbox::Event::remove function pointer with converted signature") {
		firstCalls = 0;
		secondCalls = 0;

		rawrbox::Event<std::string> a;
		a += firstRef;
		a += secondRef;

		a -= secondRef;
		REQUIRE(a.size() == 1);

		a("ok");
		REQUIRE(firstCalls == 1);
		REQUIRE(secondCalls == 0);
	}

	SECTION("rawrbox::Event::remove std::function holding a pointer") {
		firstCalls = 0;
		secondCalls = 0;

		rawrbox::Event<int> a;
		a += first;
		a += second;

		rawrbox::Event<int>::Func func = first;
		a -= func;
		REQUIRE(a.size() == 1);

		a(1);
		REQUIRE(firstCalls == 0);
		REQUIRE(secondCalls == 1);
	}

	SECTION("rawrbox::Event::remove lambda") {
		int calls = 0;
		auto lambda = [&calls](int /*_v*/) { calls++; };

		rawrbox::Event<int> a;
		a += lambda;
		a += [&calls](int /*_v*/) { calls += 10; };
		a += first;

		a -= lambda;
		REQUIRE(a.size() == 2);

		a(1);
		REQUIRE(calls == 10);
	}

	SECTION("rawrbox::Event::==") {
		rawrbox::Event<int> a;
		rawrbox::Event<int> b;

		a += first;
		b += first;

		REQUIRE(a == a);
		REQUIRE_FALSE(a != a);

		REQUIRE(a != b);
		REQUIRE_FALSE(a == b);
	}
}