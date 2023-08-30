#include <rawrbox/utils/i18n.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("I18N should behave as expected", "[rawrbox::I18N]") {
	SECTION("rawrbox::I18N::language") {
		REQUIRE(rawrbox::I18N::getLanguage() == "en");
		REQUIRE_NOTHROW(rawrbox::I18N::setLanguage("es"));
		REQUIRE(rawrbox::I18N::getLanguage() == "es");
	}

	SECTION("rawrbox::I18N::addLanguage") {
		nlohmann::json json;
		json["test"] = "hello";
		json["test2"] = "cats are awesome!";

		nlohmann::json json2;
		json2["test"] = "ola";

		REQUIRE_NOTHROW(rawrbox::I18N::addLanguagePack("pack_1", "en", json));
		REQUIRE_NOTHROW(rawrbox::I18N::addLanguagePack("pack_1", "pt", json2));

		REQUIRE(rawrbox::I18N::get("pack_3", "bla") == "$I18N FOR ID 'pack_3' NOT FOUND$");
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
