#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/scripting/mod.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {
	short priority(const std::string& json) {
		const std::string folder = "./rawrbox-tests-mod";
		std::filesystem::create_directories(folder);

		std::ofstream(folder + "/mod.json", std::ios::binary | std::ios::trunc) << json;
		return rawrbox::SCRIPTING::loadMetadata(folder).priority;
	}
} // namespace

TEST_CASE("Mod should behave as expected", "[rawrbox::Mod]") {
	SECTION("rawrbox::Mod::Mod") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};

		REQUIRE(mod.getFolder() == "./my_mod");
		REQUIRE(mod.getID() == "my_mod");
	}

	SECTION("rawrbox::Mod::init") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};
		luaL_openlibs(mod.getEnvironment());

		REQUIRE_NOTHROW(mod.init());
	}

	SECTION("rawrbox::SCRIPTING::loadMetadata") {
		REQUIRE(priority("{}") == 0);

		REQUIRE(priority(R"({"priority": 10})") == 10);
		REQUIRE(priority(R"({"priority": -5})") == -5);
		REQUIRE(priority(R"({"priority": 32767})") == 32767);
		REQUIRE(priority(R"({"priority": -32768})") == -32768);

		REQUIRE_THROWS(priority(R"({"priority": 32768})"));
		REQUIRE_THROWS(priority(R"({"priority": -32769})"));
		REQUIRE_THROWS(priority(R"({"priority": 1.5})"));
		REQUIRE_THROWS(priority(R"({"priority": "high"})"));

		REQUIRE(priority(R"({"name": "my mod", "priority": 2})") == 2);
		// ---------
		std::ofstream("./rawrbox-tests-mod/mod.json", std::ios::binary | std::ios::trunc) << R"({"name": "My Mod", "author": "FailCake", "description": "A test mod", "repo": "https://github.com/edunad/rawrbox"})";

		const auto full = rawrbox::SCRIPTING::loadMetadata("./rawrbox-tests-mod");
		REQUIRE(full.name == "My Mod");
		REQUIRE(full.author == "FailCake");
		REQUIRE(full.description == "A test mod");
		REQUIRE(full.repo == "https://github.com/edunad/rawrbox");
		REQUIRE(full.priority == 0);

		std::ofstream("./rawrbox-tests-mod/mod.json", std::ios::binary | std::ios::trunc) << R"({"name": "My Mod", "author": "FailCake"})";

		const auto minimal = rawrbox::SCRIPTING::loadMetadata("./rawrbox-tests-mod");
		REQUIRE_FALSE(minimal.description.has_value());
		REQUIRE_FALSE(minimal.repo.has_value());
		// ---------

		rawrbox::Mod mod = {"my_mod", "./my_mod", {.name = "My Mod", .priority = 3}};
		REQUIRE(mod.getMetadata().name == "My Mod");
		REQUIRE(mod.getMetadata().priority == 3);

		std::filesystem::remove_all("./rawrbox-tests-mod");
	}

	SECTION("rawrbox::Mod::load") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};
		luaL_openlibs(mod.getEnvironment());

		REQUIRE_THROWS(mod.load());

		REQUIRE_NOTHROW(mod.init());
		REQUIRE_NOTHROW(mod.script("return true"));

		REQUIRE(!mod.call("test").has_value());
		REQUIRE_NOTHROW(mod.script("function MOD:test() return true end"));

		REQUIRE(mod.call("test").has_value());
		for (size_t i = 0; i < 10000; i++)
			REQUIRE(mod.call("test").value().wasOk());

		REQUIRE_NOTHROW(mod.gc());
	}
}