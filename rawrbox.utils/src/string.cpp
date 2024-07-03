
#include <rawrbox/utils/string.hpp>

#include <algorithm>
#include <sstream>
#include <string>

namespace rawrbox {
	std::string StrUtils::toLower(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return str;
	}

	std::string StrUtils::toUpper(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
		return str;
	}

	std::string StrUtils::replace(std::string str, const std::string& find, const std::string& replace) {
		if (str.empty()) return str;

		auto fnd = str.find(find);
		if (fnd == std::string::npos) return str;

		str.replace(fnd, find.size(), replace);
		return str;
	}

	bool StrUtils::isNumeric(const std::string& str) {
		std::string::const_iterator it = str.begin();
		while (it != str.end() && (std::isdigit(*it) != 0)) {
			++it;
		}

		return !str.empty() && it == str.end();
	}

	std::string StrUtils::extractNumbers(std::string str) {
		str.erase(std::remove_if(str.begin(), str.end(), [](char c) { return std::isdigit(c) == 0; }), str.end());
		return str;
	}

	std::vector<std::string> StrUtils::split(const std::string& haystack, char needle, bool filterEmpty) {
		std::vector<std::string> elems;
		std::stringstream ss(haystack);
		std::string item;

		while (std::getline(ss, item, needle)) {
			if (filterEmpty && item.empty()) continue;
			elems.push_back(item);
		}

		return elems;
	}

	std::vector<std::string> StrUtils::split(const std::string& haystack, const std::string& needle, bool filterEmpty) {
		std::vector<std::string> result = {};

		size_t pos = 0;
		size_t found = 0;

		while ((found = haystack.find(needle, pos)) != std::string::npos) {
			std::string token = haystack.substr(pos, found - pos);
			if (!filterEmpty || !token.empty()) {
				result.push_back(token);
			}
			pos = found + needle.length();
		}

		std::string lastToken = haystack.substr(pos);
		if (!filterEmpty || !lastToken.empty()) {
			result.push_back(lastToken);
		}

		return result;
	}
} // namespace rawrbox
