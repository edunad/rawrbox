#include <rawrbox/utils/console.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CONSOLE should behave as expected", "[rawrbox::CONSOLE]") {
	std::unique_ptr<rawrbox::Console> console = std::make_unique<rawrbox::Console>();
	bool OK = false;

	SECTION("rawrbox::commands") {
		REQUIRE(console->getCommands().size() == 4); // Default commands

		console->registerCommand(
		    "test", [](const std::vector<std::string>& /*args*/) { return std::make_pair<bool, std::string>(true, ""); }, "TEST DESCRIPTION", rawrbox::ConsoleFlags::CHEAT);

		REQUIRE(console->getCommands().size() == 5);
		REQUIRE(console->hasCommand("test") == true);
		REQUIRE(console->getCommands().at("test").cheat() == true);
		REQUIRE(console->getCommands().at("test").developer() == false);
		REQUIRE(console->getCommands().at("test").description == "TEST DESCRIPTION");

		REQUIRE(console->removeCommand("test") == true);
		REQUIRE(console->removeCommand("dfds") == false);
		REQUIRE(console->getCommands().size() == 4);

		console->registerCommand("var_test", OK, "TEST DESCRIPTION", rawrbox::ConsoleFlags::DEVELOPER);

		REQUIRE(console->getCommands().size() == 5);
		REQUIRE(console->hasCommand("var_test") == true);
		REQUIRE(OK == false);

		auto ex = console->executeCommand({"var_test", "true"});
		REQUIRE(ex.first == true);
		REQUIRE(ex.second == "");
		REQUIRE(OK == true);

		REQUIRE(console->removeCommand("var_test") == true);
		REQUIRE(console->getCommands().size() == 4);
	}

	SECTION("rawrbox::print") {
		REQUIRE(console->getLogs().empty() == true);

		console->print("aaaaa");
		REQUIRE(console->getLogs().size() == 1);
		REQUIRE(console->getLogs()[0].log == "aaaaa");
		REQUIRE(console->getLogs()[0].type == rawrbox::PrintType::LOG);

		console->print("errrr", rawrbox::PrintType::ERR);
		REQUIRE(console->getLogs().size() == 2);
		REQUIRE(console->getLogs()[1].log == "errrr");
		REQUIRE(console->getLogs()[1].type == rawrbox::PrintType::ERR);

		console->clear();
		REQUIRE(console->getLogs().empty() == true);
	}

	SECTION("rawrbox::executeCommand") {
		REQUIRE(console->getLogs().empty() == true);

		auto ex = console->executeCommand({"echo"});

		REQUIRE(ex.first == false);
		REQUIRE(ex.second == "Missing argument");

		auto ex2 = console->executeCommand({"echo", "test"});
		REQUIRE(ex2.first == true);
		REQUIRE(ex2.second == "");

		REQUIRE(console->getLogs().size() == 1);
		REQUIRE(console->getLogs()[0].log == "test");
		REQUIRE(console->getLogs()[0].type == rawrbox::PrintType::ECHO);

		console->clear();
		REQUIRE(console->getLogs().empty() == true);
	}
}
