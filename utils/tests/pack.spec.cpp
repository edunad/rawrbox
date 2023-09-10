#include <rawrbox/utils/pack.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <string>

TEST_CASE("Pack utils should behave as expected", "[rawrbox::Pack]") {
	SECTION("rawrbox::packNormal") {
		uint32_t packed_1 = rawrbox::PackUtils::packNormal(0.4F);
		std::array<float, 4> unpack_1 = rawrbox::PackUtils::fromNormal(packed_1);

		uint32_t packed_2 = rawrbox::PackUtils::packNormal(0.4F, 0.23F, 0.5F);
		std::array<float, 4> unpack_2 = rawrbox::PackUtils::fromNormal(packed_2);

		REQUIRE_THAT(unpack_1[0], Catch::Matchers::WithinAbs(0.4F, 0.0001F));
		REQUIRE_THAT(unpack_1[1], Catch::Matchers::WithinAbs(0.F, 0.0001F));
		REQUIRE_THAT(unpack_1[2], Catch::Matchers::WithinAbs(0.F, 0.0001F));
		REQUIRE_THAT(unpack_1[3], Catch::Matchers::WithinAbs(0.F, 0.0001F));

		REQUIRE_THAT(unpack_2[0], Catch::Matchers::WithinAbs(0.4F, 0.0001F));
		REQUIRE_THAT(unpack_2[1], Catch::Matchers::WithinAbs(0.23F, 0.0001F));
		REQUIRE_THAT(unpack_2[2], Catch::Matchers::WithinAbs(0.5F, 0.0001F));
		REQUIRE_THAT(unpack_2[3], Catch::Matchers::WithinAbs(0.F, 0.0001F));
	}
}
