#pragma once

#include <string>
#include <vector>

namespace rawrbox {
	class StrUtils {
	public:
		static std::string toLower(std::string str);
		static std::string toUpper(std::string str);

		static bool isNumeric(const std::string& str);
		static std::string extractNumbers(std::string str);

		static std::vector<std::string> split(const std::string& haystack, char needle);
	};
} // namespace rawrbox
