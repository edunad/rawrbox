#include <rawrbox/utils/path.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <string>

TEST_CASE("PathUtils should behave as expected", "[rawrBox::PathUtils]") {
	SECTION("rawrBox::PathUtils::stripRootPath") {

#ifdef WIN32
		REQUIRE(rawrBox::PathUtils::stripRootPath("C:/windows/pls") == "pls");
		REQUIRE(rawrBox::PathUtils::stripRootPath("C:/windows/pls/screee") == "pls//screee");
#else
		REQUIRE(rawrBox::PathUtils::stripRootPath("C:/windows/pls") == "windows/pls");
		REQUIRE(rawrBox::PathUtils::stripRootPath("C:\windows\pls\screee") == "pls/screee");
#endif
	}
}
