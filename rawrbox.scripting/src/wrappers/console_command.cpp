#include <rawrbox/scripting/wrappers/console_command.hpp>
#include <rawrbox/utils/console.hpp>

namespace rawrbox {
	void ConsoleCommandWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::ConsoleCommand>("ConsoleCommand")
		    .addConstructor<void(std::string, uint32_t)>()
		    .addFunction("admin", &rawrbox::ConsoleCommand::admin)
		    .addFunction("cheat", &rawrbox::ConsoleCommand::cheat)
		    .addFunction("developer", &rawrbox::ConsoleCommand::developer)
		    .addFunction("variable", &rawrbox::ConsoleCommand::variable)
		    .endClass();
	}
} // namespace rawrbox
