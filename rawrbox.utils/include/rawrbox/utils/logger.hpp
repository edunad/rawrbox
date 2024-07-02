#pragma once

#ifdef RAWRBOX_TRACE_EXCEPTIONS
	#include <cpptrace/cpptrace.hpp>
#endif

#include <fmt/color.h>
#include <fmt/format.h>

#include <string>

namespace rawrbox {
	class Logger {
	protected:
		std::string _title = "UNKNOWN";
		bool _autoNewLine = true;

	public:
		Logger(const std::string& title);
		void setAutoNewLine(bool set);

		template <typename... T>
		void warn(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);

			fmt::print("[{} ▓ {}]: {}", fmt::styled("WARN", fmt::fg(fmt::color::yellow)), fmt::styled(this->_title, fmt::fg(fmt::color::gold)), fmt::styled(str, fmt::fg(fmt::color::yellow)));
			if (this->_autoNewLine) fmt::print("\n");
		}

#ifdef RAWRBOX_TRACE_EXCEPTIONS
		template <typename... T>
		cpptrace::runtime_error error(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);
			return cpptrace::runtime_error(fmt::format("[{} ▓ {}]: {}", fmt::styled("ERROR", fmt::fg(fmt::color::red)), fmt::styled(this->_title, fmt::fg(fmt::color::gold)), str));
		}

		template <typename... T>
		static cpptrace::runtime_error err(const std::string& title, fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);
			return cpptrace::runtime_error(fmt::format("[{} ▓ {}]: {}", fmt::styled("ERROR", fmt::fg(fmt::color::red)), fmt::styled(title, fmt::fg(fmt::color::gold)), str));
		}
#else
		template <typename... T>
		std::exception error(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);
			return std::runtime_error(fmt::format("[{} ▓ {}]: {}", fmt::styled("ERROR", fmt::fg(fmt::color::red)), fmt::styled(this->_title, fmt::fg(fmt::color::gold)), str));
		}

		template <typename... T>
		static std::exception err(const std::string& title, fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);
			return std::runtime_error(fmt::format("[{} ▓ {}]: {}", fmt::styled("ERROR", fmt::fg(fmt::color::red)), fmt::styled(title, fmt::fg(fmt::color::gold)), str));
		}
#endif

		template <typename... T>
		void printError(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);

			fmt::print("[{} ▓ {}]: {}", fmt::styled("ERROR", fmt::fg(fmt::color::red)), fmt::styled(this->_title, fmt::fg(fmt::color::gold)), str);
			if (this->_autoNewLine) fmt::print("\n");
		}

		template <typename... T>
		void info(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);

			fmt::print("[{} ▓ {}]: {}", fmt::styled("INFO", fmt::fg(fmt::color::alice_blue)), fmt::styled(this->_title, fmt::fg(fmt::color::gold)), str);
			if (this->_autoNewLine) fmt::print("\n");
		}

		template <typename... T>
		void success(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);

			fmt::print("[{} ▓ {}]: {}", fmt::styled("SUCCESS", fmt::fg(fmt::color::lime_green)), fmt::styled(this->_title, fmt::fg(fmt::color::gold)), str);
			if (this->_autoNewLine) fmt::print("\n");
		}
	};
} // namespace rawrbox
