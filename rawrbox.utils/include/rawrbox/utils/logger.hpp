#pragma once

#include <cpptrace/cpptrace.hpp>

#include <fmt/color.h>
#include <fmt/format.h>

#include <string>

namespace rawrbox {
	class Logger {
	protected:
		std::string _title = "UNKNOWN";

	public:
		Logger(const std::string& title);

		template <typename... T>
		void warn(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);
			fmt::print("[{} ▓ {}]: {}\n", fmt::format(fmt::fg(fmt::color::yellow), "WARN"), fmt::format(fmt::fg(fmt::color::gold), this->_title), fmt::format(fmt::fg(fmt::color::yellow), str));
		}

		template <typename... T>
		cpptrace::runtime_error error(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);
			return cpptrace::runtime_error(fmt::format("[{} █ {}]: {}", fmt::format(fmt::fg(fmt::color::red), "ERROR"), fmt::format(fmt::fg(fmt::color::gold), this->_title), str));
		}

		template <typename... T>
		void info(fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);
			fmt::print("[{} ▒ {}]: {}\n", fmt::format(fmt::fg(fmt::color::alice_blue), "INFO"), fmt::format(fmt::fg(fmt::color::gold), this->_title), str);
		}

		template <typename... T>
		static cpptrace::runtime_error err(const std::string& title, fmt::format_string<T...> fmt, T&&... args) {
			auto str = fmt::format(fmt, std::forward<T>(args)...);
			return cpptrace::runtime_error(fmt::format("[{} █ {}]: {}", fmt::format(fmt::fg(fmt::color::red), "ERROR"), fmt::format(fmt::fg(fmt::color::gold), title), str));
		}
	};
} // namespace rawrbox
