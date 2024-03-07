#include <rawrbox/math/utils/pack.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Pack utils should behave as expected", "[rawrbox::Pack]") {
	SECTION("rawrbox::packNormal") {
		uint32_t packed_1 = rawrbox::PackUtils::packNormal(0.4F);
		std::array<float, 4> unpack_1 = rawrbox::PackUtils::fromNormal(packed_1);

		uint32_t packed_2 = rawrbox::PackUtils::packNormal(0.4F, 0.23F, 0.5F);
		std::array<float, 4> unpack_2 = rawrbox::PackUtils::fromNormal(packed_2);

		short packed_3 = rawrbox::PackUtils::toHalf(0.43F);
		float unpack_3 = rawrbox::PackUtils::fromHalf(packed_3);

		REQUIRE_THAT(unpack_1[0], Catch::Matchers::WithinAbs(0.4F, 0.0001F));
		REQUIRE_THAT(unpack_1[1], Catch::Matchers::WithinAbs(0.F, 0.0001F));
		REQUIRE_THAT(unpack_1[2], Catch::Matchers::WithinAbs(0.F, 0.0001F));
		REQUIRE_THAT(unpack_1[3], Catch::Matchers::WithinAbs(0.F, 0.0001F));

		REQUIRE_THAT(unpack_2[0], Catch::Matchers::WithinAbs(0.4F, 0.0001F));
		REQUIRE_THAT(unpack_2[1], Catch::Matchers::WithinAbs(0.23F, 0.0001F));
		REQUIRE_THAT(unpack_2[2], Catch::Matchers::WithinAbs(0.5F, 0.0001F));
		REQUIRE_THAT(unpack_2[3], Catch::Matchers::WithinAbs(0.F, 0.0001F));

		REQUIRE_THAT(unpack_3, Catch::Matchers::WithinAbs(0.43F, 0.0001F));
	}

	SECTION("rawrbox::packColor") {
		uint32_t id = (1 << 8) | 0xFF;

		auto packed_1 = rawrbox::PackUtils::fromRGBA(id);

		REQUIRE_THAT(packed_1[0], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(packed_1[1], Catch::Matchers::WithinAbs(0.F, 0.0001F));
		REQUIRE_THAT(packed_1[2], Catch::Matchers::WithinAbs(0.00392156886F, 0.0001F));
		REQUIRE_THAT(packed_1[3], Catch::Matchers::WithinAbs(1.F, 0.0001F));

		auto unpacked_1 = rawrbox::PackUtils::toRGBA(packed_1[0], packed_1[1], packed_1[2], 1.F);
		REQUIRE(unpacked_1 == id);

		auto unpacked_2 = rawrbox::PackUtils::toRGBA(static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(1), static_cast<uint8_t>(255));
		REQUIRE(unpacked_2 == id);
	}
}
