#include <rawrbox/utils/json.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("JSONUtils should behave as expected", "[rawrbox::JSONUtils]") {

	glz::generic tr1 = {};
	REQUIRE(glz::read_json(tr1, R"({"test":"hello", "oke": 34, "data": {"knife": true, "keep": true}, "data2": [{"g": 4}]})") == glz::error_code::none);

	glz::generic tr2 = {};
	REQUIRE(glz::read_json(tr2, R"({"test":"ola", "data": {"knife": false, "keep": true}, "data2": [{"g": 5}, {"d": 2}]})") == glz::error_code::none);

	SECTION("rawrbox::JSONUtils::diff") {
		auto diff = rawrbox::JSONUtils::diff(tr1, tr2);
		REQUIRE(diff.size() == 5);

		REQUIRE(diff[0].op == rawrbox::JSONDiffOp::REPLACE);
		REQUIRE(diff[0].path == "/data/knife");
		REQUIRE(diff[0].value.get<bool>() == false);

		REQUIRE(diff[1].op == rawrbox::JSONDiffOp::REPLACE);
		REQUIRE(diff[1].path == "/data2/0/g");
		REQUIRE(diff[1].value.get<double>() == 5);

		REQUIRE(diff[2].op == rawrbox::JSONDiffOp::ADD);
		REQUIRE(diff[2].path == "/data2/1");

		REQUIRE(diff[3].op == rawrbox::JSONDiffOp::REMOVE);
		REQUIRE(diff[3].path == "/oke");

		REQUIRE(diff[4].op == rawrbox::JSONDiffOp::REPLACE);
		REQUIRE(diff[4].path == "/test");
		REQUIRE(diff[4].value.get<std::string>() == "ola");
	}

	SECTION("rawrbox::JSONUtils::patch") {
		auto diff = rawrbox::JSONUtils::diff(tr1, tr2);
		REQUIRE(diff.size() == 5);

		glz::generic tr3 = tr1;
		REQUIRE_NOTHROW(rawrbox::JSONUtils::patch(tr3, diff));

		REQUIRE(tr3.contains("data"));
		REQUIRE(tr3.contains("data2"));
		REQUIRE(tr3.contains("test"));
		REQUIRE(!tr3.contains("oke"));

		REQUIRE(tr3["test"].get<std::string>() == "ola");

		REQUIRE(tr3["data"].contains("knife"));
		REQUIRE(tr3["data"]["knife"].get<bool>() == false);

		REQUIRE(tr3["data2"][0].contains("g"));
		REQUIRE(tr3["data2"][1].contains("d"));
		REQUIRE(tr3["data2"][0]["g"].get<double>() == 5);
		REQUIRE(tr3["data2"][1]["d"].get<double>() == 2);
	}
}
