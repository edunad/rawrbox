#include <rawrbox/math/easing.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Easing functions are correctly implemented", "[Easing]") {
	SECTION("rawrbox::Easing::LINEAR") {
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::LINEAR, 0.5F), Catch::Matchers::WithinAbs(0.5F, 0.0001F));
	}

	SECTION("rawrbox::Easing::STEP") {
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::STEP, 0.49F), Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::STEP, 0.51F), Catch::Matchers::WithinAbs(1.0F, 0.0001F));
	}

	SECTION("rawrbox::Easing::EASE_IN_QUAD") {
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::EASE_IN_QUAD, 0.5F), Catch::Matchers::WithinAbs(0.25F, 0.0001F));
	}

	SECTION("rawrbox::Easing::EASE_OUT_QUAD") {
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::EASE_OUT_QUAD, 0.5F), Catch::Matchers::WithinAbs(0.75F, 0.0001F));
	}

	SECTION("rawrbox::Easing::EASE_IN_OUT_QUAD") {
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::EASE_IN_OUT_QUAD, 0.25F), Catch::Matchers::WithinAbs(0.125F, 0.0001F));
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::EASE_IN_OUT_QUAD, 0.75F), Catch::Matchers::WithinAbs(0.875F, 0.0001F));
	}

	SECTION("rawrbox::Easing::EASE_IN_CUBIC") {
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::EASE_IN_CUBIC, 0.5F), Catch::Matchers::WithinAbs(0.125F, 0.0001F));
	}

	SECTION("rawrbox::Easing::EASE_OUT_CUBIC") {
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::EASE_OUT_CUBIC, 0.5F), Catch::Matchers::WithinAbs(0.875F, 0.0001F));
	}

	SECTION("rawrbox::Easing::EASE_IN_OUT_CUBIC") {
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::EASE_IN_OUT_CUBIC, 0.25F), Catch::Matchers::WithinAbs(0.0625F, 0.0001F));
		REQUIRE_THAT(rawrbox::EasingUtils::ease(rawrbox::Easing::EASE_IN_OUT_CUBIC, 0.75F), Catch::Matchers::WithinAbs(0.9375F, 0.0001F));
	}
}
