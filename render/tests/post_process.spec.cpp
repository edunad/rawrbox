#include <rawrbox/render/postprocess/bloom.hpp>
#include <rawrbox/render/postprocess/manager.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <bx/math.h>

#include <string>

#include "rawrbox/math/vector2.hpp"

TEST_CASE("PostProcessManager should behave as expected", "[rawrBox::PostProcessManager]") {

	SECTION("rawrBox::PostProcessManager::add / rawrBox::PostProcessManager::count") {
		auto base = std::make_shared<rawrBox::PostProcessManager>(0, rawrBox::Vector2i{100, 100});

		REQUIRE(base->count() == 0);
		base->add(std::make_shared<rawrBox::PostProcessBloom>(0.001F));
		REQUIRE(base->count() == 1);
	}

	SECTION("rawrBox::PostProcessManager::remove") {
		auto base = std::make_shared<rawrBox::PostProcessManager>(0, rawrBox::Vector2i{100, 100});

		REQUIRE(base->count() == 0);
		REQUIRE_THROWS(base->remove(3));

		base->add(std::make_shared<rawrBox::PostProcessBloom>(0.001F));
		REQUIRE(base->count() == 1);
		REQUIRE_THROWS(base->remove(3));

		REQUIRE_NOTHROW(base->remove(0));
		REQUIRE(base->count() == 0);
	}

	SECTION("rawrBox::PostProcessManager::get") {
		auto base = std::make_shared<rawrBox::PostProcessManager>(0, rawrBox::Vector2i{100, 100});

		REQUIRE_THROWS(base->get(23));
		base->add(std::make_shared<rawrBox::PostProcessBloom>(0.001F));
		REQUIRE_THROWS(base->get(23));
		REQUIRE_NOTHROW(base->get(0));
	}
}
