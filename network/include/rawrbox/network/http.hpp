
#pragma once

#include <cpr/cpr.h>

#include <functional>
#include <map>
#include <string>

namespace rawrbox {
	enum class HTTPMethod {
		GET = 0,
		POST,
		PUT,
		ERASE, // because DELETED
		OPTIONS
	};

	class HTTP {
	public:
		static void request(const std::string& url, const HTTPMethod method, const std::map<std::string, std::string>& headers, std::function<void(int, std::map<std::string, std::string>, std::string)> callback, int timeout = 10000);
	};
} // namespace rawrbox
