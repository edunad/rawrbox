#pragma once

#include <string>
#include <vector>

namespace rawrbox {
	class StrUtils {
	public:
		static std::string toLower(std::string str);
		static std::string toUpper(std::string str);
		static std::string replace(std::string str, const std::string& find, const std::string& replace);
		static std::string truncate(std::string str, size_t length);

		static bool isNumeric(const std::string& str);
		static std::string extractNumbers(std::string str);

		static std::vector<std::string> split(const std::string& haystack, const std::string& needle, bool filterEmpty = false);
		static std::vector<std::string> split(const std::string& haystack, char needle, bool filterEmpty = false);
	};
} // namespace rawrbox
