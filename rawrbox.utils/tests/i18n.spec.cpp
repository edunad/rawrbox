#include <rawrbox/utils/i18n.hpp>

#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

TEST_CASE("I18N should behave as expected", "[rawrbox::I18N]") {
	SECTION("rawrbox::I18N::language") {
		REQUIRE(rawrbox::I18N::getLanguage() == "en");
		REQUIRE_NOTHROW(rawrbox::I18N::setLanguage("es"));
		REQUIRE(rawrbox::I18N::getLanguage() == "es");
	}

	SECTION("rawrbox::I18N::addLanguage") {
		std::string testTranslation = R"({"test":"hello", "test2":"cats are awesome!"})";
		std::string testTranslation2 = R"({"test":"ola"})";

		rawrbox::Translation tr1 = {};
		REQUIRE(glz::read_json(tr1, testTranslation) == glz::error_code::none);

		rawrbox::Translation tr2 = {};
		REQUIRE(glz::read_json(tr2, testTranslation2) == glz::error_code::none);

		REQUIRE_NOTHROW(rawrbox::I18N::setLanguage("en"));

		REQUIRE_NOTHROW(rawrbox::I18N::addToLanguagePack("pack_1", "en", tr1));
		REQUIRE_NOTHROW(rawrbox::I18N::addToLanguagePack("pack_1", "pt", tr2));

		REQUIRE(rawrbox::I18N::get("pack_3", "bla") == "$I18N FOR LANGUAGE 'pack_3-en' NOT FOUND$");
		REQUIRE(rawrbox::I18N::get("pack_1", "bla") == "$I18N KEY 'bla' NOT FOUND$");

		REQUIRE_NOTHROW(rawrbox::I18N::setLanguage("en"));
		REQUIRE(rawrbox::I18N::get("pack_1", "test") == "hello");
		REQUIRE_NOTHROW(rawrbox::I18N::setLanguage("pt"));
		REQUIRE(rawrbox::I18N::get("pack_1", "test") == "ola");
		REQUIRE(rawrbox::I18N::get("pack_1", "test2") == "$I18N KEY 'test2' NOT FOUND$");
		REQUIRE_NOTHROW(rawrbox::I18N::setLanguage("cat"));
		REQUIRE(rawrbox::I18N::get("pack_1", "test2") == "cats are awesome!"); // use en fallback
	}

	SECTION("rawrbox::I18N::find") {
		REQUIRE_NOTHROW(rawrbox::I18N::setLanguage("en"));
		REQUIRE_NOTHROW(rawrbox::I18N::addToLanguagePack("pack_find", "en", {{"count", "{} cats"}, {"plain", "cats"}}));

		REQUIRE(rawrbox::I18N::find("pack_find", "plain") == "cats");
		REQUIRE(rawrbox::I18N::find("pack_find", "count", {"3"}) == "3 cats");
		REQUIRE_FALSE(rawrbox::I18N::find("pack_find", "missing").has_value());
		REQUIRE_FALSE(rawrbox::I18N::find("pack_none", "plain").has_value());
	}

	SECTION("rawrbox::I18N::parseKey") {
		REQUIRE(rawrbox::I18N::parseKey("#menu.food")->key == "menu.food");
		REQUIRE(rawrbox::I18N::parseKey("#menu.food")->id.empty());
		REQUIRE(rawrbox::I18N::parseKey("#coremod:menu.food")->id == "coremod");
		REQUIRE(rawrbox::I18N::parseKey("#coremod:menu.food")->key == "menu.food");

		REQUIRE_FALSE(rawrbox::I18N::parseKey("menu.food").has_value());
		REQUIRE_FALSE(rawrbox::I18N::parseKey("#").has_value());
		REQUIRE_FALSE(rawrbox::I18N::parseKey("#/textures/a.png").has_value());
		REQUIRE_FALSE(rawrbox::I18N::parseKey("#two words").has_value());
		REQUIRE_FALSE(rawrbox::I18N::parseKey("#:key").has_value());
		REQUIRE_FALSE(rawrbox::I18N::parseKey("#id:").has_value());
		REQUIRE_FALSE(rawrbox::I18N::parseKey("#a:b:c").has_value());
	}

	SECTION("rawrbox::I18N::resolve") {
		REQUIRE_NOTHROW(rawrbox::I18N::setLanguage("en"));
		REQUIRE_NOTHROW(rawrbox::I18N::addToLanguagePack("pack_a", "en", {{"name", "Cat"}, {"count", "{} cats"}}));
		REQUIRE_NOTHROW(rawrbox::I18N::addToLanguagePack("pack_b", "en", {{"name", "Dog"}}));

		REQUIRE(rawrbox::I18N::resolve("plain text", "pack_a") == "plain text");
		REQUIRE(rawrbox::I18N::resolve("#name", "pack_a") == "Cat");
		REQUIRE(rawrbox::I18N::resolve("#pack_b:name", "pack_a") == "Dog");
		REQUIRE(rawrbox::I18N::resolve("#count", "pack_a", {"2"}) == "2 cats");
		REQUIRE(rawrbox::I18N::resolve("#missing", "pack_a") == "$I18N KEY 'missing' NOT FOUND$");
	}
}
