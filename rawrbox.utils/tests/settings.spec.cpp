#include <rawrbox/utils/settings.hpp>

#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

struct TestSettingsData {
	std::string VERSION = "0.0.2";

	std::map<std::string, float> video = {{"scale", 0.5F}, {"monitor", 2.F}};
	std::map<std::string, std::string> controls = {{"jump", "A"}, {"crouch", "L_CNTRL"}};

	std::string username = "MISSING";
};

class TestSettings : public rawrbox::Settings<TestSettingsData> {
public:
	[[nodiscard]] std::filesystem::path getFileName() const override { return "meow.json"; }
	void save() override{}; // Prevent saving the file
};

TEST_CASE("Settings should behave as expected", "[rawrbox::Settings]") {
	std::unique_ptr<TestSettings> _settings = std::make_unique<TestSettings>();

	SECTION("rawrbox::Settings::load") {
		REQUIRE_NOTHROW(_settings->load(R"({"video":{"scale":0.3,"monitor":-3},"steam_id":"aaaaaaaaaaaaaaaaaaaaaaa","controls":{"jump":"B"},"VERSION":"0.0.1"})"));
		auto& settings = _settings->getSettings();

		REQUIRE(settings.video.contains("scale"));
		REQUIRE(settings.video["scale"] == 0.3F);
		REQUIRE(settings.video.contains("monitor"));
		REQUIRE(settings.video["monitor"] == -3.F);

		REQUIRE(settings.controls.contains("jump"));
		REQUIRE(settings.controls["jump"] == "B");

		REQUIRE(settings.controls.contains("crouch"));
		REQUIRE(settings.controls["crouch"] == "L_CNTRL");
		REQUIRE(settings.username == "MISSING");

		REQUIRE(settings.VERSION == "0.0.2");
	}

	SECTION("rawrbox::Settings::load::defaults") {
		REQUIRE_NOTHROW(_settings->load(""));
		auto& settings = _settings->getSettings();

		REQUIRE(settings.video.contains("scale"));
		REQUIRE(settings.video["scale"] == 0.5F);
		REQUIRE(settings.video.contains("monitor"));
		REQUIRE(settings.video["monitor"] == 2.F);

		REQUIRE(settings.controls.contains("jump"));
		REQUIRE(settings.controls["jump"] == "A");

		REQUIRE(settings.controls.contains("crouch"));
		REQUIRE(settings.controls["crouch"] == "L_CNTRL");

		REQUIRE(settings.username == "MISSING");
		REQUIRE(settings.VERSION == "0.0.2");
	}
}
