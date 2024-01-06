

#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/console_wrapper.hpp>

namespace rawrbox {
	ConsoleWrapper::ConsoleWrapper(rawrbox::Console* console) : _console(console) {}
	ConsoleWrapper::~ConsoleWrapper() { this->_console = nullptr; }

	void ConsoleWrapper::clear() {
		if (this->_console == nullptr) throw std::runtime_error("[RawrBox-ConsoleWrapper] Console instance not set!");
		this->_console->clear();
	}

	std::pair<bool, std::string> ConsoleWrapper::execute(const std::vector<std::string>& args) {
		if (this->_console == nullptr) throw std::runtime_error("[RawrBox-ConsoleWrapper] Console instance not set!");
		return this->_console->executeCommand(args);
	}

	void ConsoleWrapper::print(const std::string& text, sol::optional<rawrbox::PrintType> type) {
		if (this->_console == nullptr) throw std::runtime_error("[RawrBox-ConsoleWrapper] Console instance not set!");
		this->_console->print(text, type.value_or(rawrbox::PrintType::LOG));
	}

	const rawrbox::ConsoleCommand& ConsoleWrapper::get(const std::string& command) {
		if (this->_console == nullptr) throw std::runtime_error("[RawrBox-ConsoleWrapper] Console instance not set!");
		return this->_console->getCommand(command);
	}

	bool ConsoleWrapper::registerMethod(const std::string& command, sol::function callback, sol::optional<std::string> description, sol::optional<uint32_t> flags) {
		if (this->_console == nullptr) throw std::runtime_error("[RawrBox-ConsoleWrapper] Console instance not set!");
		return this->_console->registerCommand(
		    command, [callback](const std::vector<std::string>& args) {
			    auto ret = rawrbox::LuaUtils::runCallback(callback, args);

			    switch (ret.return_count()) {
				    case 2: return std::make_pair<bool, const std::string&>(ret[0].get<bool>(), ret[1].get<std::string>());
				    case 1: return std::make_pair<bool, const std::string&>(ret[0].get<bool>(), "");
				    default: return std::make_pair<bool, const std::string&>(true, "");
			    }
		    },
		    description.value_or(""), flags.value_or(rawrbox::ConsoleFlags::NONE));
	}

	bool ConsoleWrapper::registerVariable(const std::string& /*command*/, sol::object /*variable*/, sol::optional<std::string> /*description*/, sol::optional<uint32_t> /*flags*/) {
		throw std::runtime_error("TODO");
	}

	void ConsoleWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ConsoleWrapper>("console",
		    sol::no_constructor,

		    "clear", &ConsoleWrapper::clear,
		    "execute", &ConsoleWrapper::execute,
		    "print", &ConsoleWrapper::print,

		    "get", &ConsoleWrapper::get,

		    "registerMethod", &ConsoleWrapper::registerMethod,
		    "registerVariable", &ConsoleWrapper::registerVariable);
	}
} // namespace rawrbox
