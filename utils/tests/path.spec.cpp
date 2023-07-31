#include <rawrbox/utils/path.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("PathUtils should behave as expected", "[rawrbox::PathUtils]") {
	SECTION("rawrbox::PathUtils::stripRootPath") {
#ifdef WIN32
		REQUIRE(rawrbox::PathUtils::stripRootPath("C:/windows/pls") == "pls");
		REQUIRE(rawrbox::PathUtils::stripRootPath("C:/windows/pls/screee") == "pls//screee");
#else
		REQUIRE(rawrbox::PathUtils::stripRootPath("C:/windows/pls") == "windows/pls");
		REQUIRE(rawrbox::PathUtils::stripRootPath("C:/windows/pls/screee") == "windows/pls/screee");
#endif
	}
}
