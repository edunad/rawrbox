
#include <rawrbox/utils/string.hpp>

#include <algorithm>
#include <sstream>

namespace rawrbox {
	std::string StrUtils::toLower(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}

	std::string StrUtils::toUpper(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		return str;
	}

	bool StrUtils::isNumeric(const std::string& str) {
		std::string::const_iterator it = str.begin();

		while (it != str.end() && std::isdigit(*it))
			++it;
		return !str.empty() && it == str.end();
	}

	std::string StrUtils::extractNumbers(std::string str) {
		str.erase(std::remove_if(str.begin(), str.end(), [](char c) { return !std::isdigit(c); }), str.end());
		return str;
	}

	std::vector<std::string> StrUtils::split(const std::string& haystack, char needle) {
		std::vector<std::string> elems;
		std::stringstream ss(haystack);
		std::string item;

		while (std::getline(ss, item, needle)) {
			elems.push_back(item);
		}

		return elems;
	}
} // namespace rawrbox
