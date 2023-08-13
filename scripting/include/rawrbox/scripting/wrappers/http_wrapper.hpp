#pragma once

#include <rawrbox/utils/http.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class HTTPWrapper {
	public:
		HTTPWrapper() = default;
		HTTPWrapper(const HTTPWrapper&) = default;
		HTTPWrapper(HTTPWrapper&&) = default;
		HTTPWrapper& operator=(const HTTPWrapper&) = default;
		HTTPWrapper& operator=(HTTPWrapper&&) = default;
		virtual ~HTTPWrapper() = default;

		// UTILS -----
		virtual void request(const std::string& url, const rawrbox::HTTPMethod method, sol::table headers, sol::function callback, sol::optional<int> timeout);
		// ----------------

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
