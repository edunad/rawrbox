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

		REQUIRE_THAT(unpack_1[0], Catch::Matchers::WithinAbs(0.4F, 0.0001F));
		REQUIRE_THAT(unpack_1[1], Catch::Matchers::WithinAbs(0.F, 0.0001F));
		REQUIRE_THAT(unpack_1[2], Catch::Matchers::WithinAbs(0.F, 0.0001F));
		REQUIRE_THAT(unpack_1[3], Catch::Matchers::WithinAbs(0.F, 0.0001F));

		REQUIRE_THAT(unpack_2[0], Catch::Matchers::WithinAbs(0.4F, 0.0001F));
		REQUIRE_THAT(unpack_2[1], Catch::Matchers::WithinAbs(0.23F, 0.0001F));
		REQUIRE_THAT(unpack_2[2], Catch::Matchers::WithinAbs(0.5F, 0.0001F));
		REQUIRE_THAT(unpack_2[3], Catch::Matchers::WithinAbs(0.F, 0.0001F));
	}

	SECTION("rawrbox::FP16") {
		short packed_3 = rawrbox::PackUtils::toFP16(0.43F);
		float unpack_3 = rawrbox::PackUtils::fromFP16(packed_3);

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

	SECTION("rawrbox::packBones") {
		auto indices = rawrbox::PackUtils::packBoneIndices({0, 3, 149, 255});
		REQUIRE(indices == std::array<uint8_t, 4>{0, 3, 149, 255});

		auto weights = rawrbox::PackUtils::packBoneWeights({1.F, 0.5F, 0.F, 2.F}); // 0 -> 1 max
		REQUIRE(weights[0] == 65535);
		REQUIRE(weights[1] == 32768);
		REQUIRE(weights[2] == 0);
		REQUIRE(weights[3] == 65535); // Clamped
	}

	SECTION("rawrbox::packOctNormal") {
		const std::array<std::array<float, 3>, 6> normals = {{
		    {0.F, 1.F, 0.F},
		    {0.F, 0.F, -1.F},
		    {0.57735F, -0.57735F, 0.57735F},
		    {-0.26726F, 0.53452F, -0.80178F},
		    {1.F, 0.F, 0.F},
		    {-0.70711F, 0.F, -0.70711F},
		}};

		for (const auto& n : normals) {
			auto unpacked = rawrbox::PackUtils::fromOCTNormal(rawrbox::PackUtils::packOCTNormal(n[0], n[1], n[2]));

			REQUIRE_THAT(unpacked[0], Catch::Matchers::WithinAbs(n[0], 0.0005F));
			REQUIRE_THAT(unpacked[1], Catch::Matchers::WithinAbs(n[1], 0.0005F));
			REQUIRE_THAT(unpacked[2], Catch::Matchers::WithinAbs(n[2], 0.0005F));
		}

		auto zero = rawrbox::PackUtils::fromOCTNormal(rawrbox::PackUtils::packOCTNormal(0.F, 0.F, 0.F));
		REQUIRE_THAT(zero[2], Catch::Matchers::WithinAbs(1.F, 0.0005F));
	}

	SECTION("rawrbox::packOctTangent") {
		auto positive = rawrbox::PackUtils::fromOCTTangent(rawrbox::PackUtils::packOCTTangent(0.57735F, -0.57735F, -0.57735F, 1.F));

		REQUIRE_THAT(positive[0], Catch::Matchers::WithinAbs(0.57735F, 0.001F));
		REQUIRE_THAT(positive[1], Catch::Matchers::WithinAbs(-0.57735F, 0.001F));
		REQUIRE_THAT(positive[2], Catch::Matchers::WithinAbs(-0.57735F, 0.001F));

		REQUIRE(positive[3] == 1.F);

		auto negative = rawrbox::PackUtils::fromOCTTangent(rawrbox::PackUtils::packOCTTangent(1.F, 0.F, 0.F, -1.F));

		REQUIRE_THAT(negative[0], Catch::Matchers::WithinAbs(1.F, 0.001F));
		REQUIRE_THAT(negative[1], Catch::Matchers::WithinAbs(0.F, 0.001F));
		REQUIRE_THAT(negative[2], Catch::Matchers::WithinAbs(0.F, 0.001F));

		REQUIRE(negative[3] == -1.F);
	}
}
