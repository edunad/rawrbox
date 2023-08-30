#pragma once
#include <rawrbox/utils/console.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class ConsoleWrapper {
		rawrbox::Console* _console = nullptr;

	public:
		ConsoleWrapper(rawrbox::Console* console);
		ConsoleWrapper(const ConsoleWrapper&) = default;
		ConsoleWrapper(ConsoleWrapper&&) = default;
		ConsoleWrapper& operator=(const ConsoleWrapper&) = default;
		ConsoleWrapper& operator=(ConsoleWrapper&&) = default;
		virtual ~ConsoleWrapper();

		virtual void clear();
		virtual bool registerMethod(const std::string& command, sol::function callback, sol::optional<std::string> description, sol::optional<uint32_t> flags);
		virtual bool registerVariable(const std::string& command, sol::object variable, sol::optional<std::string> description, sol::optional<uint32_t> flags);
		virtual void print(const std::string& command, sol::optional<rawrbox::PrintType> type);

		virtual std::pair<bool, std::string> execute(const std::vector<std::string>& args);
		virtual const rawrbox::ConsoleCommand& get(const std::string& command);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
