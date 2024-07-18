#include <rawrbox/utils/console.hpp>
#include <rawrbox/utils/string.hpp>

#include <fmt/ranges.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace rawrbox {
	Console::Console() {
		// Register default commands
		this->registerCommand(
		    "find", [this](const std::vector<std::string>& args) {
			    if (args.empty() || args.size() <= 1) return std::make_pair<bool, std::string>(false, "Missing command argument");

			    std::unordered_map<std::string, ConsoleCommand> cmds = this->findCommand(args[1]);
			    if (cmds.empty()) return std::make_pair<bool, std::string>(false, fmt::format("Failed to find commmand {}", args[1]));

			    std::string helper;
			    for (auto& cmd : cmds) {
				    helper += fmt::format("[#1abc9c] **{}** [/][#ffffff]-> {}[/]\n", cmd.first, cmd.second.description);
			    }

			    this->print(helper, PrintType::LOG);
			    return std::make_pair<bool, std::string>(true, "");
		    },
		    "Finds the given command and prints the description");

		this->registerCommand(
		    "clear", [this](const std::vector<std::string>& /*args*/) {
			    this->clear();
			    return std::make_pair<bool, std::string>(true, "");
		    },
		    "Clears the console");

		this->registerCommand(
		    "list", [this](const std::vector<std::string>& /*args*/) {
			    for (const auto& cmd : getCommands()) {
				    if (!cmd.second.variable()) continue;
				    this->print(fmt::format("{}: [#ffffff]{}[/] ", cmd.first, this->getCommandValue(cmd.second)), PrintType::LOG);
			    }

			    return std::make_pair<bool, std::string>(true, "");
		    },
		    "Lists the console current variable values");

		this->registerCommand(
		    "echo", [this](const std::vector<std::string>& args) {
			    if (args.empty() || args.size() <= 1) return std::make_pair<bool, std::string>(false, "Missing argument");

			    std::vector<std::string> cleanArgs = args;
			    cleanArgs.erase(cleanArgs.begin());

			    this->print(fmt::format("{}", fmt::join(cleanArgs, " ")), PrintType::ECHO);
			    return std::make_pair<bool, std::string>(true, "");
		    },
		    "Prints on the console");
	}

	// CFG ----
	bool Console::executeCFG(const std::string& id) {
		auto p = std::filesystem::path(fmt::format("cfg/{}.cfg", id));
		if (!std::filesystem::exists(p)) return false;

		// Load
		std::ifstream filehandle(p.generic_string());
		if (!filehandle.is_open())
			throw std::runtime_error(fmt::format("[Console] Failed to find cfg 'cfg/{}.cfg'", id));

		std::stringstream strCmds;
		strCmds << filehandle.rdbuf();

		this->print(fmt::format("Executing CFG 'cfg/{}.cfg'", id));

		std::string cmd;
		while (std::getline(strCmds, cmd)) {
			if (cmd.empty() || cmd.starts_with("//")) continue;
			this->printExecuteCommand(rawrbox::StrUtils::split(cmd, ' '));
		}

		return true;
	}
	// -------------

	std::unordered_map<std::string, ConsoleCommand> Console::findCommand(const std::string& findCmd, unsigned int limit) {
		std::unordered_map<std::string, ConsoleCommand> foundCmds;
		for (auto& cmd : this->_commands) {
			if (limit > 0 && foundCmds.size() >= limit) return foundCmds;
			if (cmd.first.find(findCmd) == std::string::npos) continue;

			foundCmds.emplace(cmd);
		}

		return foundCmds;
	}

	std::string Console::getCommandValue(const ConsoleCommand& cmd) const {
		if (!cmd.variable()) return "function";

		const auto& var = cmd.var.value();
		if (std::holds_alternative<int>(var)) return fmt::format("{}", std::get<int>(var));
		if (std::holds_alternative<int*>(var)) return fmt::format("{}", *std::get<int*>(var));

		if (std::holds_alternative<size_t>(var)) return fmt::format("{}", std::get<size_t>(var));
		if (std::holds_alternative<size_t*>(var)) return fmt::format("{}", *std::get<size_t*>(var));

		if (std::holds_alternative<float>(var)) return fmt::format("{}", std::get<float>(var));
		if (std::holds_alternative<float*>(var)) return fmt::format("{}", *std::get<float*>(var));

		if (std::holds_alternative<bool>(var)) return fmt::format("{}", std::get<bool>(var));
		if (std::holds_alternative<bool*>(var)) return fmt::format("{}", *std::get<bool*>(var));

		if (std::holds_alternative<std::string>(var)) return fmt::format("{}", std::get<std::string>(var));
		if (std::holds_alternative<std::string*>(var)) return fmt::format("{}", *std::get<std::string*>(var));

		throw std::runtime_error("Unknown command value");
	}

	const std::unordered_map<std::string, ConsoleCommand>& Console::getCommands() const {
		return this->_commands;
	}

	std::vector<std::string> Console::getCommandStr() const {
		std::vector<std::string> cmds;
		cmds.reserve(this->_commands.size());

		for (const auto& pair : this->_commands) {
			cmds.push_back(pair.first);
		}

		return cmds;
	}

	void Console::print(const std::string& message, PrintType type) {
		if (message.empty()) return;

		// encase in scope for the lock only to be there while printing to std::out and appending the log vector
		{
			const std::lock_guard<std::mutex> lock(this->_logLock);

			this->_log.emplace_back(message, type);
			if (this->_log.size() > 30) this->_log.erase(this->_log.begin());

			switch (type) {
				case PrintType::WARNING:
					fmt::print("[WARNING] {}\n", message);
					break;
				case PrintType::ERR:
					fmt::print("[ERROR] {}\n", message);
					break;
				case PrintType::EXECUTE:
					fmt::print("[COMMAND] {}\n", message);
					break;
				case PrintType::ECHO:
					fmt::print("[ECHO] {}\n", message);
					break;
				default:
					fmt::print("[LOG] {}\n", message);
					break;
			}
		}

		onPrint();
	}

	void Console::clear() {
		const std::lock_guard<std::mutex> lock(this->_logLock);
		this->_log.clear();
	}

	std::vector<ConsoleLog> Console::getLogs() {
		const std::lock_guard<std::mutex> lock(this->_logLock);
		return this->_log;
	}

	bool Console::hasCommand(const std::string& cmd) const {
		return this->_commands.find(cmd) != this->_commands.end();
	}

	const rawrbox::ConsoleCommand& Console::getCommand(const std::string& id) {
		auto fnd = this->_commands.find(id);
		if (fnd == this->_commands.end()) throw std::runtime_error(fmt::format("[RawrBox-Console] Command '{}' not found!", id));

		return fnd->second;
	}

	std::pair<bool, std::string> Console::run(const std::vector<std::string>& args) {
		const auto& cmd = args[0];
		auto& command = this->_commands[cmd];

		// easiest flow
		if (command.method.has_value()) {
			onExecute(cmd, args);
			return command.method.value()(args);
		}

		// set variable
		if (command.var.has_value() && args.size() > 1) {
			auto& var = command.var.value();

			try {
				if (std::holds_alternative<int>(var)) var = std::stoi(args[1]);
				if (std::holds_alternative<int*>(var)) *std::get<int*>(var) = std::stoi(args[1]);

				if (std::holds_alternative<size_t>(var)) var = std::stoi(args[1]);
				if (std::holds_alternative<size_t*>(var)) *std::get<size_t*>(var) = std::stoull(args[1]);

				if (std::holds_alternative<float>(var)) var = std::stof(args[1]);
				if (std::holds_alternative<float*>(var)) *std::get<float*>(var) = std::stof(args[1]);

				if (std::holds_alternative<bool>(var)) var = args[1] == "false" ? false : args[1] == "true" ? true
															    : std::stoi(args[1]) > 0;
				if (std::holds_alternative<bool*>(var)) *std::get<bool*>(var) = args[1] == "false" ? false : args[1] == "true" ? true
																	       : std::stoi(args[1]) > 0;

				if (std::holds_alternative<std::string>(var)) var = args[1];
				if (std::holds_alternative<std::string*>(var)) *std::get<std::string*>(var) = args[1];
			} catch (const std::runtime_error&) {
				if (std::holds_alternative<int>(var) || std::holds_alternative<int*>(var)) return {false, "invalid input, expected non-decimal number"};
				if (std::holds_alternative<bool>(var) || std::holds_alternative<bool*>(var)) return {false, "invalid input, expected true, false or non-decimal number higher than zero"};
			}
		}

		this->onExecute(cmd, args);
		return {true, ""};
	}

	void Console::printExecuteCommand(const std::vector<std::string>& args) {
		if (args.empty()) return;
		if (args[0] != "echo") this->print(fmt::format("> {}", fmt::join(args, " ")), rawrbox::PrintType::EXECUTE);

		auto resp = this->executeCommand(args);
		this->print(resp.second, resp.first ? rawrbox::PrintType::LOG : rawrbox::PrintType::ERR);
	}

	std::pair<bool, std::string> Console::executeCommand(const std::vector<std::string>& args) {
		if (args.empty()) return {false, "Command not found"};

		const auto& cmd = args[0];
		if (cmd.empty() || !hasCommand(cmd)) return {false, "Command not found"};

		if (this->validate != nullptr) {
			return this->validate(this->_commands[cmd]);
		}

		return run(args);
	}

	bool Console::removeCommand(const std::string& id) {
		auto fnd = this->_commands.find(id);
		if (fnd == this->_commands.end()) return false;
		this->_commands.erase(fnd);

		return true;
	}
} // namespace rawrbox
