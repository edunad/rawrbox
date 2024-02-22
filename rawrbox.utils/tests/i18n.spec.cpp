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
}
