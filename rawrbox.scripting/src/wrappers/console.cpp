

#include <rawrbox/scripting/wrappers/console.hpp>

namespace rawrbox {
	// PRIVATE ---
	rawrbox::Console* ConsoleWrapper::_console = nullptr;
	// ------------

	void ConsoleWrapper::init(rawrbox::Console* console) { _console = console; }
	void ConsoleWrapper::shutdown() { _console = nullptr; }

	// UTILS ---
	void ConsoleWrapper::clear() {
		if (_console == nullptr) throw std::runtime_error("Console instance not set!");
		_console->clear();
	}

	const rawrbox::ConsoleCommand& ConsoleWrapper::get(const std::string& command) {
		if (_console == nullptr) throw std::runtime_error("Console instance not set!");
		return _console->getCommand(command);
	}
	// ---------

	std::pair<bool, std::string> ConsoleWrapper::execute(lua_State* L) {
		if (_console == nullptr) throw std::runtime_error("Console instance not set!");
		return _console->executeCommand(rawrbox::LuaUtils::argsToString(L, true));
	}

	void ConsoleWrapper::print(const std::string& text, std::optional<rawrbox::PrintType> type) {
		if (_console == nullptr) throw std::runtime_error("Console instance not set!");
		_console->print(text, type.value_or(rawrbox::PrintType::LOG));
	}

	bool ConsoleWrapper::registerMethod(const std::string& command, const luabridge::LuaRef& callback, const std::optional<std::string>& description, std::optional<uint32_t> flags) {
		if (_console == nullptr) throw std::runtime_error("Console instance not set!");
		if (!callback.isCallable()) throw std::runtime_error("Invalid callback argument");

		return _console->registerCommand(
		    command, [callback](const std::vector<std::string>& args) -> std::pair<bool, std::string> {
			    auto tbl = rawrbox::LuaUtils::vectorToTable(callback.state(), args);
			    auto ret = luabridge::call(callback, tbl);
			    if (ret.hasFailed()) fmt::print("Lua error\n  └── {}\n", ret.errorMessage());

			    switch (ret.size()) {
				    case 2: return {ret[0].template unsafe_cast<bool>(), ret[1].template unsafe_cast<std::string>()};
				    case 1: return {ret[0].template unsafe_cast<bool>(), ""};
				    default: return {false, ""};
			    }
		    },
		    description.value_or(""), flags.value_or(rawrbox::ConsoleFlags::NONE));
	}

	bool ConsoleWrapper::removeCommand(const std::string& command) {
		if (_console == nullptr) throw std::runtime_error("Console instance not set!");
		return _console->removeCommand(command);
	}

	bool ConsoleWrapper::hasCommand(const std::string& command) {
		if (_console == nullptr) throw std::runtime_error("Console instance not set!");
		return _console->hasCommand(command);
	}

	void ConsoleWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("console", {})
		    // UTILS ----
		    .addFunction("clear", &ConsoleWrapper::clear)
		    .addFunction("get", &ConsoleWrapper::get)
		    // ------
		    .addFunction("execute", &ConsoleWrapper::execute)
		    .addFunction("print", &ConsoleWrapper::print)
		    .addFunction("registerMethod", &ConsoleWrapper::registerMethod)
		    .addFunction("removeCommand", &ConsoleWrapper::removeCommand)
		    .addFunction("hasCommand", &ConsoleWrapper::hasCommand)
		    .endNamespace();
	}
} // namespace rawrbox
