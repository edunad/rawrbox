#include <rawrbox/utils/path.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <string>

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

	SECTION("rawrbox::PathUtils::matchGlob") {
		REQUIRE(rawrbox::PathUtils::matchGlob("textures/ui.png", "textures/ui.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("textures/ui.png", "textures/ui.pngx"));

		REQUIRE(rawrbox::PathUtils::matchGlob("textures/*.png", "textures/ui.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("textures/*.png", "textures/ui/overlay.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("*.png", "textures/ui.png"));

		REQUIRE(rawrbox::PathUtils::matchGlob("**/*.png", "ui.png"));
		REQUIRE(rawrbox::PathUtils::matchGlob("**/*.png", "textures/ui/overlay.png"));
		REQUIRE(rawrbox::PathUtils::matchGlob("**/*norm.png", "textures/light_test/planksnorm.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("**/a.png", "textures/xa.png"));

		REQUIRE(rawrbox::PathUtils::matchGlob("shaders/**", "shaders/include/common.fxh"));
		REQUIRE(rawrbox::PathUtils::matchGlob("textures/**/overlay.png", "textures/overlay.png"));
		REQUIRE(rawrbox::PathUtils::matchGlob("textures/**/overlay.png", "textures/ui/overlay/overlay.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("shaders/**", "textures/shaders/a.png"));

		REQUIRE(rawrbox::PathUtils::matchGlob("textures/?.png", "textures/a.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("textures?a.png", "textures/a.png"));

		REQUIRE(rawrbox::PathUtils::matchGlob("textures/*", "textures/"));
		REQUIRE(rawrbox::PathUtils::matchGlob("*a", "aaa"));
		REQUIRE(rawrbox::PathUtils::matchGlob("*_norm*.png", "planks_norm_norm2.png"));
		REQUIRE(rawrbox::PathUtils::matchGlob("assets/**/textures/*.png", "assets/a/textures/b/textures/c.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("assets/**/textures/*.png", "assets/a/textures/b/c.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("**/a/*.png", "b/a.png"));

		REQUIRE(rawrbox::PathUtils::matchGlob("tex**.png", "textures.png"));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("tex**.png", "textures/a/b.png"));
		REQUIRE(rawrbox::PathUtils::matchGlob("*b**/a.png", "abb/a.png"));
		REQUIRE(rawrbox::PathUtils::matchGlob("a/***/b", "a/x/y/b"));

		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("*a*a*a*a*a*a*a*a*a*a*b", std::string(512, 'a')));
		REQUIRE_FALSE(rawrbox::PathUtils::matchGlob("**/a**/a**/a**/a**/a**/b", std::string(256, 'a') + "/" + std::string(256, 'a')));
	}
}