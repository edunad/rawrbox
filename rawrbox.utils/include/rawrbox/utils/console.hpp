#pragma once

#include <rawrbox/utils/event.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>

namespace rawrbox {
	namespace ConsoleFlags {
		const uint32_t NONE = 0;
		const uint32_t CHEAT = 1 << 1;
		const uint32_t ADMIN = 1 << 2;
		const uint32_t DEVELOPER = 1 << 3;
	}; // namespace ConsoleFlags

	enum class PrintType {
		// Internal
		EXECUTE,
		ECHO,

		LOG,
		WARNING,
		ERR
	};

	using ConsoleFunction = std::function<std::pair<bool, std::string>(const std::vector<std::string>& args)>;
	using ConsoleVariable = std::variant<int*, int, size_t*, size_t, std::string*, std::string, bool*, bool, float, float*>;

	class ConsoleCommand {
	public:
		std::optional<ConsoleVariable> var;
		std::optional<ConsoleFunction> method;

		std::string description;
		uint32_t flags = ConsoleFlags::NONE;

		[[nodiscard]] virtual bool admin() const {
			return (flags & ConsoleFlags::ADMIN) > 0;
		};

		[[nodiscard]] virtual bool developer() const {
			return (flags & ConsoleFlags::DEVELOPER) > 0;
		};

		[[nodiscard]] virtual bool cheat() const {
			return (flags & ConsoleFlags::CHEAT) > 0;
		};

		[[nodiscard]] virtual bool variable() const {
			return var.has_value();
		};

		ConsoleCommand(std::string desc, uint32_t flags) : description(std::move(desc)), flags(flags){};
		ConsoleCommand() = default;
		ConsoleCommand(const ConsoleCommand&) = default;
		ConsoleCommand(ConsoleCommand&&) = default;
		ConsoleCommand& operator=(const ConsoleCommand&) = default;
		ConsoleCommand& operator=(ConsoleCommand&&) = default;
		virtual ~ConsoleCommand() = default;
	};

	class ConsoleLog {
	public:
		PrintType type;

		std::string timestamp;
		std::string log;

		ConsoleLog(std::string log_, rawrbox::PrintType type_) : type(type_), log(std::move(log_)) {
			const std::time_t t_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			std::stringstream ss;
			ss << std::put_time(std::localtime(&t_c), "%T");
			timestamp = ss.str();
		};

		ConsoleLog(const ConsoleLog&) = default;
		ConsoleLog(ConsoleLog&&) = default;
		ConsoleLog& operator=(const ConsoleLog&) = default;
		ConsoleLog& operator=(ConsoleLog&&) = default;
		~ConsoleLog() = default;
	};

	class Console {
		std::vector<rawrbox::ConsoleLog> _log = {};
		std::mutex _logLock = {};
		std::unordered_map<std::string, ConsoleCommand> _commands = {};

		std::pair<bool, std::string> run(const std::vector<std::string>& args);

	public:
		rawrbox::Event<const std::string&, const std::vector<std::string>&> onExecute;
		rawrbox::Event<> onPrint;

		std::function<std::pair<bool, std::string>(const rawrbox::ConsoleCommand&)> validate = nullptr;

		Console();
		Console(const Console&) = delete;
		Console(Console&&) = delete;
		Console& operator=(const Console&) = delete;
		Console& operator=(Console&&) = delete;
		virtual ~Console() = default;

		virtual std::vector<rawrbox::ConsoleLog> getLogs();
		virtual void print(const std::string& message, PrintType type = PrintType::LOG);
		virtual void clear();

		// COMMANDS ----
		[[nodiscard]] virtual const std::unordered_map<std::string, ConsoleCommand>& getCommands() const;
		[[nodiscard]] virtual std::string getCommandValue(const ConsoleCommand& cmd) const;
		[[nodiscard]] virtual std::vector<std::string> getCommandStr() const;

		virtual std::unordered_map<std::string, rawrbox::ConsoleCommand> findCommand(const std::string& cmd, unsigned int limit = 0);

		[[nodiscard]] virtual const rawrbox::ConsoleCommand& getCommand(const std::string& id);
		[[nodiscard]] virtual bool hasCommand(const std::string& cmd) const;
		// -------------

		// CFG ----
		virtual bool executeCFG(const std::string& id);
		// -------------

		virtual void printExecuteCommand(const std::vector<std::string>& args);
		virtual std::pair<bool, std::string> executeCommand(const std::vector<std::string>& args);

		virtual bool removeCommand(const std::string& id);

		bool registerCommand(std::string key, ConsoleFunction func, std::string description = "", uint32_t flags = ConsoleFlags::NONE) {
			std::transform(key.begin(), key.end(), key.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });
			if (_commands.find(key) != _commands.end()) {
				print(fmt::format("Console command `{}` has already been registered.", key), PrintType::ERR);
				return false;
			}

			_commands[key] = ConsoleCommand(description, flags);
			_commands[key].method = func;

			return true;
		}

		template <typename T>
		bool registerCommand(std::string key, T& ref, std::string description = "", uint32_t flags = ConsoleFlags::NONE) {
			std::transform(key.begin(), key.end(), key.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });
			if (_commands.find(key) != _commands.end()) {
				print(fmt::format("Console command `{}` has already been registered.", key), PrintType::ERR);
				return false;
			}

			_commands[key] = ConsoleCommand(description, flags);
			_commands[key].var = &ref;

			return true;
		}
	};
} // namespace rawrbox
