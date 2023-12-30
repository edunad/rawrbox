#include <rawrbox/math/color.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Color should behave as expected", "[rawrbox::Color]") {

	SECTION("rawrbox::Color") {
		rawrbox::Colori col = rawrbox::Colori{255, 255, 0, 255};

		REQUIRE(col.r == 255);
		REQUIRE(col.g == 255);
		REQUIRE(col.b == 0);
		REQUIRE(col.a == 255);
	}

	SECTION("rawrbox::Colors") {
		auto red = rawrbox::Colors::Red();

		REQUIRE(red.r == 1.F);
		REQUIRE(red.g == 0.32F);
		REQUIRE(red.b == 0.32F);
		REQUIRE(red.a == 1.F);
	}

	SECTION("rawrbox::Color::cast") {
		auto red = rawrbox::Colors::Red().cast<int>();
		rawrbox::Colori col = rawrbox::Colori{255, 255, 0, 255};
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

	SECTION("rawrbox::Color::strength") {
		auto red = rawrbox::Colors::Red().cast<int>().strength(0.5F);

		REQUIRE(red.r == 127);
		REQUIRE(red.g == 40);
		REQUIRE(red.b == 40);
		REQUIRE(red.a == 255);
	}

	SECTION("rawrbox::Color::debug") {
		auto cl1 = rawrbox::Colori::debug(2);
		auto cl2 = rawrbox::Colorf::debug(13);

		REQUIRE(cl1.r == 52);
		REQUIRE(cl1.g == 172);
		REQUIRE(cl1.b == 224);
		REQUIRE(cl1.a == 255);

		REQUIRE_THAT(cl2.r, Catch::Matchers::WithinAbs(0.20392F, 0.0001F));
		REQUIRE_THAT(cl2.g, Catch::Matchers::WithinAbs(0.67451F, 0.0001F));
		REQUIRE_THAT(cl2.b, Catch::Matchers::WithinAbs(0.87843F, 0.0001F));
		REQUIRE_THAT(cl2.a, Catch::Matchers::WithinAbs(1.F, 0.0001F));
	}

	SECTION("rawrbox::Color::isTransparent") {
		rawrbox::Colori col = rawrbox::Colori{255, 0, 255, 255};
		rawrbox::Colori col2 = rawrbox::Colori{255, 255, 0, 0};

		REQUIRE(col.isTransparent() == false);
		REQUIRE(col2.isTransparent() == true);
	}

	SECTION("rawrbox::Color::lerp") {
		rawrbox::Colori col = rawrbox::Colori(255, 0, 255, 255).lerp({255, 255, 255, 255}, 0.5F);

		REQUIRE(col.r == 255);
		REQUIRE(col.g == 127);
		REQUIRE(col.b == 255);
		REQUIRE(col.a == 255);
	}

	SECTION("rawrbox::Color::pack") {
		rawrbox::Colori col = rawrbox::Colori(255, 0, 255, 255);
		REQUIRE(col.pack() == 0xffff00ff);
	}

	SECTION("rawrbox::Color::RGBHex") {
		rawrbox::Colori col = rawrbox::Colori::RGBHex(0xffff00);

		REQUIRE(col.r == 255);
		REQUIRE(col.g == 255);
		REQUIRE(col.b == 0);
		REQUIRE(col.a == 255);
	}

	SECTION("rawrbox::Color::dot") {
		int col = rawrbox::Colori(255, 0, 255, 255).dot({255, 255, 255, 255});
		REQUIRE(col == 195075);
	}

	SECTION("rawrbox::Color::toLinear") {
		auto col = rawrbox::Colors::Blue().toLinear();

		REQUIRE_THAT(col.r, Catch::Matchers::WithinAbs(0.0331F, 0.0001F));
		REQUIRE_THAT(col.g, Catch::Matchers::WithinAbs(0.40645F, 0.0001F));
		REQUIRE_THAT(col.b, Catch::Matchers::WithinAbs(0.72935F, 0.0001F));
		REQUIRE_THAT(col.a, Catch::Matchers::WithinAbs(1.F, 0.0001F));
	}

	SECTION("rawrbox::Color::toLinear") {
		auto col = rawrbox::Color(0.0331F, 0.40645F, 0.72935F, 1.F).toSRGB();
		auto col2 = rawrbox::Colors::Blue();

		REQUIRE_THAT(col.r, Catch::Matchers::WithinAbs(col2.r, 0.0001F));
		REQUIRE_THAT(col.g, Catch::Matchers::WithinAbs(col2.g, 0.0001F));
		REQUIRE_THAT(col.b, Catch::Matchers::WithinAbs(col2.b, 0.0001F));
		REQUIRE_THAT(col.a, Catch::Matchers::WithinAbs(col2.a, 0.0001F));
	}
}
