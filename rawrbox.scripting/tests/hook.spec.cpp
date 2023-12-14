#include <rawrbox/scripting/hooks.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("Hook should behave as expected", "[rawrbox::Hook]") {
	std::unique_ptr<rawrbox::Hooks> hooks = std::make_unique<rawrbox::Hooks>();

	SECTION("rawrbox::Hook") {
		REQUIRE(hooks->empty() == true);
		REQUIRE(hooks->count() == 0);

		REQUIRE_NOTHROW(hooks->listen("test", "test-hook", {}));
		REQUIRE(hooks->count() == 1);

		REQUIRE_NOTHROW(hooks->remove("test34", "test-hoo23k"));
		REQUIRE(hooks->count() == 1);

		REQUIRE_NOTHROW(hooks->remove("test", "test-hook"));
		REQUIRE(hooks->count() == 0);
	}
}
