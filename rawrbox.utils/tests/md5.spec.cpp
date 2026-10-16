#include <rawrbox/utils/md5.hpp>

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <string>

namespace {
	std::string hash(const std::string& str) {
		return rawrbox::MD5::toString(rawrbox::MD5::calculate(std::bit_cast<const uint8_t*>(str.data()), str.size()));
	}
} // namespace

TEST_CASE("MD5 should behave as expected", "[rawrbox::MD5]") {
	SECTION("rawrbox::MD5::calculate") {
		REQUIRE(hash("") == "d41d8cd98f00b204e9800998ecf8427e");
		REQUIRE(hash("a") == "0cc175b9c0f1b6a831c399e269772661");
		REQUIRE(hash("abc") == "900150983cd24fb0d6963f7d28e17f72");
		REQUIRE(hash("message digest") == "f96b697d7cb7938d525a2f31aaf161d0");
		REQUIRE(hash("abcdefghijklmnopqrstuvwxyz") == "c3fcd3d76192e4007dfb496cca67e13b");
		REQUIRE(hash("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") == "d174ab98d277d9f5a5611c2c9f419d9f");
		REQUIRE(hash("12345678901234567890123456789012345678901234567890123456789012345678901234567890") == "57edf4a22be3c955ac49da2e2107b67a");
	}

	SECTION("rawrbox::MD5::update") {
		rawrbox::MD5 md5 = {};

		std::string data(1000, '\0');
		for (size_t i = 0; i < data.size(); i++) {
			data[i] = static_cast<char>(i * 31);
		}

		size_t offset = 0;
		for (size_t size : {1, 63, 64, 65, 7, 300, 500}) {
			md5.update(std::bit_cast<const uint8_t*>(data.data()) + offset, size);
			offset += size;
		}

		REQUIRE(offset == data.size());
		REQUIRE(rawrbox::MD5::toString(md5.finalize()) == hash(data));
	}
}