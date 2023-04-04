#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

namespace rawrbox::utils {
	class StrUtils {
		public:
			static std::string toLower(std::string str) {
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				return str;
			}

			static std::string toUpper(std::string str) {
				std::transform(str.begin(), str.end(), str.begin(), ::toupper);
				return str;
			}

			static bool isNumeric(const std::string& str) {
				std::string::const_iterator it = str.begin();

				while (it != str.end() && std::isdigit(*it)) ++it;
				return !str.empty() && it == str.end();
			}

			static std::string extractNumbers(std::string str) {
				str.erase(std::remove_if(str.begin(), str.end(), [](char c) { return !std::isdigit(c); }), str.end());
				return str;
			}

			static std::vector<std::string> split(const std::string& haystack, char needle) {
				std::vector<std::string> elems;
				std::stringstream ss(haystack);
				std::string item;

				while (std::getline(ss, item, needle)) {
					elems.push_back(item);
				}

				return elems;
			}
	};
}
