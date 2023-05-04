#include <rawrbox/math/color.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Color should behave as expected", "[rawrBox::Color]") {

	SECTION("rawrBox::Color") {
		rawrBox::Colori col = rawrBox::Colori{255, 255, 0, 255};

		REQUIRE(col.r == 255);
		REQUIRE(col.g == 255);
		REQUIRE(col.b == 0);
		REQUIRE(col.a == 255);
	}

	SECTION("rawrBox::Colors") {
		auto red = rawrBox::Colors::Red;

		REQUIRE(red.r == 1.F);
		REQUIRE(red.g == 0.32F);
		REQUIRE(red.b == 0.32F);
		REQUIRE(red.a == 1.F);
	}

	SECTION("rawrBox::Color::cast") {
		auto red = rawrBox::Colors::Red.cast<int>();
		rawrBox::Colori col = rawrBox::Colori{255, 255, 0, 255};
		auto clCast = col.cast<float>();

		REQUIRE(red.r == 255);
		REQUIRE(red.g == 81);
		REQUIRE(red.b == 81);
		REQUIRE(red.a == 255);

		REQUIRE(clCast.r == 1.F);
		REQUIRE(clCast.g == 1.F);
		REQUIRE(clCast.b == 0.F);
		REQUIRE(clCast.a == 1.F);
	}

	SECTION("rawrBox::Color::debug") {
		auto cl1 = rawrBox::Colori::debug(2);
		auto cl2 = rawrBox::Colorf::debug(13);

		REQUIRE(cl1.r == 52);
		REQUIRE(cl1.g == 172);
		REQUIRE(cl1.b == 224);
		REQUIRE(cl1.a == 255);

		REQUIRE_THAT(cl2.r, Catch::Matchers::WithinAbs(0.20392F, 0.0001F));
		REQUIRE_THAT(cl2.g, Catch::Matchers::WithinAbs(0.67451F, 0.0001F));
		REQUIRE_THAT(cl2.b, Catch::Matchers::WithinAbs(0.87843F, 0.0001F));
		REQUIRE_THAT(cl2.a, Catch::Matchers::WithinAbs(1.F, 0.0001F));
	}

	SECTION("rawrBox::Color::isTransparent") {
		rawrBox::Colori col = rawrBox::Colori{255, 0, 255, 255};
		rawrBox::Colori col2 = rawrBox::Colori{255, 255, 0, 0};

		REQUIRE(col.isTransparent() == false);
		REQUIRE(col2.isTransparent() == true);
	}

	SECTION("rawrBox::Color::lerp") {
		rawrBox::Colori col = rawrBox::Colori(255, 0, 255, 255).lerp({255, 255, 255, 255}, 0.5F);

		REQUIRE(col.r == 255);
		REQUIRE(col.g == 127);
		REQUIRE(col.b == 255);
		REQUIRE(col.a == 255);
	}

	SECTION("rawrBox::Color::pack") {
		rawrBox::Colori col = rawrBox::Colori(255, 0, 255, 255);
		REQUIRE(rawrBox::Colori::toHEX(col) == 0xff00ffff);
	}

	SECTION("rawrBox::Color::RGBHex") {
		rawrBox::Colori col = rawrBox::Colori::RGBHex(0xffff00);

		REQUIRE(col.r == 255);
		REQUIRE(col.g == 255);
		REQUIRE(col.b == 0);
		REQUIRE(col.a == 255);
	}

	SECTION("rawrBox::Color::dot") {
		int col = rawrBox::Colori(255, 0, 255, 255).dot({255, 255, 255, 255});
		REQUIRE(col == 195075);
	}
}
