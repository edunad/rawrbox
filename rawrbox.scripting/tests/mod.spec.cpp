#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/scripting/mod.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace rawrbox {
	static short readModPriority(const std::string& json) {
		const std::string folder = "./rawrbox-tests-mod";
		std::filesystem::create_directories(folder);

		std::ofstream(folder + "/mod.json", std::ios::binary | std::ios::trunc) << json;
		const auto metadata = rawrbox::SCRIPTING::loadMetadata(folder);
		if (!metadata) throw std::runtime_error(metadata.error());

		return metadata->priority;
	}
} // namespace rawrbox

TEST_CASE("Mod should behave as expected", "[rawrbox::Mod]") {
	SECTION("rawrbox::Mod::Mod") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};

		REQUIRE(mod.getFolder() == "./my_mod");
		REQUIRE(mod.getID() == "my_mod");
		REQUIRE(Luau::CodeGen::isNativeExecutionEnabled(mod.getEnvironment()));
	}

	SECTION("rawrbox::Mod::init") {
		rawrbox::Mod mod = {"my_mod", "./my_mod", {}};
		luaL_openlibs(mod.getEnvironment());

		REQUIRE_NOTHROW(mod.init());
	}

	SECTION("rawrbox::SCRIPTING::loadMetadata") {
		REQUIRE(rawrbox::readModPriority("{}") == 0);

		REQUIRE(rawrbox::readModPriority(R"({"priority": 10})") == 10);
		REQUIRE(rawrbox::readModPriority(R"({"priority": -5})") == -5);
		REQUIRE(rawrbox::readModPriority(R"({"priority": 32767})") == 32767);
		REQUIRE(rawrbox::readModPriority(R"({"priority": -32768})") == -32768);

		REQUIRE_THROWS(rawrbox::readModPriority(R"({"priority": 32768})"));
		REQUIRE_THROWS(rawrbox::readModPriority(R"({"priority": -32769})"));
		REQUIRE_THROWS(rawrbox::readModPriority(R"({"priority": 1.5})"));
		REQUIRE_THROWS(rawrbox::readModPriority(R"({"priority": "high"})"));

		REQUIRE(rawrbox::readModPriority(R"({"name": "my mod", "priority": 2})") == 2);
		// ---------
		std::ofstream("./rawrbox-tests-mod/mod.json", std::ios::binary | std::ios::trunc) << R"({"name": "My Mod", "author": "FailCake", "description": "A test mod", "repo": "https://github.com/edunad/rawrbox"})";

		const auto full = rawrbox::SCRIPTING::loadMetadata("./rawrbox-tests-mod");
		REQUIRE(full.has_value());
		REQUIRE(full->name == "My Mod");
		REQUIRE(full->author == "FailCake");
		REQUIRE(full->description == "A test mod");
		REQUIRE(full->repo == "https://github.com/edunad/rawrbox");
		REQUIRE(full->priority == 0);

		std::ofstream("./rawrbox-tests-mod/mod.json", std::ios::binary | std::ios::trunc) << R"({"name": "My Mod", "author": "FailCake"})";

		const auto minimal = rawrbox::SCRIPTING::loadMetadata("./rawrbox-tests-mod");
		REQUIRE(minimal.has_value());
		REQUIRE_FALSE(minimal->description.has_value());
		REQUIRE_FALSE(minimal->repo.has_value());
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

	SECTION("rawrbox::ScriptingSettings::libraries") {
		const std::string folder = "./rawrbox-tests-libraries";
		std::filesystem::create_directories(folder);

		std::ofstream(folder + "/init.luau", std::ios::binary | std::ios::trunc) << R"(
			local missing = hooks == nil and timer == nil and time == nil and engine == nil and mod == nil and json == nil and Vector2 == nil and string.vformat == nil
			local present = print ~= nil and include ~= nil and i18n ~= nil and i18n.get ~= nil and i18n.find ~= nil and i18n.setLanguage ~= nil
			RESULT = (missing and present) and 1 or 0
		)";

		rawrbox::SCRIPTING::init({.libraries = rawrbox::ScriptingLibraries::I18N});
		REQUIRE(rawrbox::SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::I18N));
		REQUIRE_FALSE(rawrbox::SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::HOOKS));
		REQUIRE_FALSE(rawrbox::SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::I18N | rawrbox::ScriptingLibraries::HOOKS));

		auto* mod = rawrbox::SCRIPTING::loadMod("libraries", folder);
		REQUIRE(mod != nullptr);
		REQUIRE(rawrbox::SCRIPTING::getMod(mod->getEnvironment()) == mod);

		lua_getglobal(mod->getEnvironment(), "RESULT");
		REQUIRE(lua_tonumber(mod->getEnvironment(), -1) == 1);
		lua_pop(mod->getEnvironment(), 1);

		REQUIRE(rawrbox::SCRIPTING::unloadMod(std::string("libraries")));
		rawrbox::SCRIPTING::init({});
		REQUIRE(rawrbox::SCRIPTING::hasLibrary(rawrbox::ScriptingLibraries::ALL));

		std::filesystem::remove_all(folder);
	}
}