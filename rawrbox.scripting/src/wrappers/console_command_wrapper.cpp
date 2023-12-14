#include <rawrbox/scripting/wrappers/console_command_wrapper.hpp>
#include <rawrbox/utils/console.hpp>

namespace rawrbox {
	void ConsoleCommandWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ConsoleCommand>("ConsoleCommand",
		    sol::constructors<ConsoleCommand(std::string, uint32_t)>(),

		    "admin", &ConsoleCommand::admin,
		    "cheat", &ConsoleCommand::cheat,
		    "developer", &ConsoleCommand::developer,
		    "variable", &ConsoleCommand::variable);
	}
} // namespace rawrbox
