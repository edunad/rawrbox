#include <rawrbox/utils/settings.hpp>

#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

class TestSettings : public rawrbox::Settings {
public:
	glz::json_t getDefaults() override {
		return {
		    {"video", {{"scale", 0.5F}, {"monitor", 2}}},
		    {"controls", {{"jump", "A"}, {"crouch", "L_CNTRL"}}},
		    {"username", "failcake"}};
	}

	[[nodiscard]] std::string getVersion() const override { return "0.0.2"; }
	[[nodiscard]] std::string getFileName() const override { return "meow.json"; }
	void save() override{}; // Prevent saving the file
};

TEST_CASE("Settings should behave as expected", "[rawrbox::Settings]") {
	std::unique_ptr<TestSettings> _settings = std::make_unique<TestSettings>();

	SECTION("rawrbox::Settings::getVersion") {
		REQUIRE(_settings->getVersion() == "0.0.2");
	}

	SECTION("rawrbox::Settings::load") {
		REQUIRE_NOTHROW(_settings->load(R"({"video":{"scale":0.3,"monitor":-3},"steam_id":"aaaaaaaaaaaaaaaaaaaaaaa","controls":{"jump":"B"},"VERSION":"0.0.1"})"));

		auto& settings = _settings->getSettings();

		REQUIRE(settings.contains("video"));
		REQUIRE(settings.contains("controls"));
		REQUIRE(settings.contains("username"));
		REQUIRE(!settings.contains("steam_id"));
		REQUIRE(settings.contains("VERSION"));

		REQUIRE(settings["video"].contains("scale"));
		REQUIRE(settings["video"]["scale"].get<double>() == 0.3);
		REQUIRE(settings["video"].contains("monitor"));
		REQUIRE(settings["video"]["monitor"].get<double>() == -3);

		REQUIRE(settings["controls"]["jump"].get<std::string>() == "B");
		REQUIRE(settings["controls"]["crouch"].get<std::string>() == "L_CNTRL");
		REQUIRE(settings["VERSION"].get<std::string>() == "0.0.2");
	}

	SECTION("rawrbox::Settings::load::defaults") {
		REQUIRE_NOTHROW(_settings->load(""));
		auto& settings = _settings->getSettings();

		REQUIRE(settings.contains("video"));
		REQUIRE(settings.contains("controls"));
		REQUIRE(settings.contains("username"));
		REQUIRE(settings.contains("VERSION"));

		REQUIRE(settings["video"].contains("scale"));
		REQUIRE(settings["video"]["scale"].get<double>() == 0.5);
		REQUIRE(settings["video"].contains("monitor"));
		REQUIRE(settings["video"]["monitor"].get<double>() == 2);

		REQUIRE(settings["controls"]["jump"].get<std::string>() == "A");
		REQUIRE(settings["controls"]["crouch"].get<std::string>() == "L_CNTRL");

		REQUIRE(settings["username"].get<std::string>() == "failcake");
		REQUIRE(settings["VERSION"].get<std::string>() == "0.0.2");
	}
}
