#include <rawrbox/utils/crc.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE("CRC should behave as expected", "[rawrbox::CRC]") {
	SECTION("rawrbox::CRC") {
		std::string buff = "test";
		auto c1 = CRC::Calculate(buff.data(), buff.size(), CRC::CRC_32());

		std::string buff2 = "test";
		auto c2 = CRC::Calculate(buff2.data(), buff2.size(), CRC::CRC_32());

		std::string buff3 = "tests";
		auto c3 = CRC::Calculate(buff3.data(), buff3.size(), CRC::CRC_32());

		REQUIRE(c1 == c2);
		REQUIRE(c1 != c3);
	}
}
