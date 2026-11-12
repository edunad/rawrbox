#include <rawrbox/utils/md5.hpp>

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <string>

namespace rawrbox {
	static std::string hashMD5(const std::string& str) {
		return rawrbox::MD5::toString(rawrbox::MD5::calculate(std::bit_cast<const uint8_t*>(str.data()), str.size()));
	}
} // namespace rawrbox

TEST_CASE("MD5 should behave as expected", "[rawrbox::MD5]") {
	SECTION("rawrbox::MD5::calculate") {
		REQUIRE(rawrbox::hashMD5("") == "d41d8cd98f00b204e9800998ecf8427e");
		REQUIRE(rawrbox::hashMD5("a") == "0cc175b9c0f1b6a831c399e269772661");
		REQUIRE(rawrbox::hashMD5("abc") == "900150983cd24fb0d6963f7d28e17f72");
		REQUIRE(rawrbox::hashMD5("message digest") == "f96b697d7cb7938d525a2f31aaf161d0");
		REQUIRE(rawrbox::hashMD5("abcdefghijklmnopqrstuvwxyz") == "c3fcd3d76192e4007dfb496cca67e13b");
		REQUIRE(rawrbox::hashMD5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") == "d174ab98d277d9f5a5611c2c9f419d9f");
		REQUIRE(rawrbox::hashMD5("12345678901234567890123456789012345678901234567890123456789012345678901234567890") == "57edf4a22be3c955ac49da2e2107b67a");
	}

	SECTION("rawrbox::MD5::calculate (padding)") {
		REQUIRE(rawrbox::hashMD5(std::string(55, 'a')) == "ef1772b6dff9a122358552954ad0df65");
		REQUIRE(rawrbox::hashMD5(std::string(56, 'a')) == "3b0c8ac703f828b04c6c197006d17218");
		REQUIRE(rawrbox::hashMD5(std::string(57, 'a')) == "652b906d60af96844ebd21b674f35e93");
		REQUIRE(rawrbox::hashMD5(std::string(63, 'a')) == "b06521f39153d618550606be297466d5");
		REQUIRE(rawrbox::hashMD5(std::string(64, 'a')) == "014842d480b571495a4a0363793f7367");
		REQUIRE(rawrbox::hashMD5(std::string(65, 'a')) == "c743a45e0d2e6a95cb859adae0248435");
		REQUIRE(rawrbox::hashMD5(std::string(119, 'a')) == "8a7bd0732ed6a28ce75f6dabc90e1613");
		REQUIRE(rawrbox::hashMD5(std::string(120, 'a')) == "5f61c0ccad4cac44c75ff505e1f1e537");
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
		REQUIRE(rawrbox::MD5::toString(md5.finalize()) == rawrbox::hashMD5(data));
	}
}