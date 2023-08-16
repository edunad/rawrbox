#include <rawrbox/scripting/scripting.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/scripting/wrappers/http_wrapper.hpp>

namespace rawrbox {
	void HTTPWrapper::request(const std::string& url, const rawrbox::HTTPMethod method, sol::table headers, sol::function callback, sol::optional<int> timeout) {
		if (url.empty()) {
			rawrbox::LuaUtils::runCallback(callback, true, "URL cannot be empty");
			return;
		}

		// Setup headers -------
		std::map<std::string, std::string> headerMap = {};
		for (auto& pair : headers) {
			std::string nameCheck = pair.first.as<std::string>();
			std::transform(nameCheck.begin(), nameCheck.end(), nameCheck.begin(), ::toupper);

			std::string name = pair.first.as<std::string>();
			std::string value = pair.second.as<std::string>();

			if (nameCheck.compare("METHOD") == 0 || nameCheck.compare("USER-AGENT") == 0) continue; // Remove these
			headerMap[name] = value;
		}
		// ------------------

		rawrbox::HTTP::request(
		    url, method, headerMap, [callback](int code, std::map<std::string, std::string> headerResp, std::string resp) {
			    if (code == 0) {
				    rawrbox::LuaUtils::runCallback(callback, true, resp); // curl error
				    return;
			    }

			    auto& lua = rawrbox::SCRIPTING::getLUA();
			    sol::table tbl = lua.create_table();
			    sol::table headerTbl = lua.create_table();

			    for (auto& pair : headerResp) {
				    headerTbl[pair.first] = pair.second;
			    }

			    tbl["status"] = code;
			    tbl["data"] = resp;
			    tbl["headers"] = headerTbl;

			    rawrbox::LuaUtils::runCallback(callback, false, tbl);
		    },
		    timeout.value_or(5000));
	}

	void HTTPWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<HTTPWrapper>("HTTP",
		    sol::no_constructor,
		    "request", &HTTPWrapper::request);
	}
} // namespace rawrbox
