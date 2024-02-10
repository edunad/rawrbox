#include <rawrbox/engine/static.hpp>
#include <rawrbox/network/scripting/wrappers/http.hpp>

namespace rawrbox {
	void HTTPWrapper::request(const std::string& url, int method, const luabridge::LuaRef& headers, const luabridge::LuaRef& callback, std::optional<int> timeout) {
		if (url.empty()) throw std::runtime_error("URL cannot be empty");
		if (!headers.isTable()) throw std::runtime_error("Invalid header table");
		if (!callback.isCallable()) throw std::runtime_error("Invalid callback");

		auto L = headers.state();

		// Setup headers -------
		std::map<std::string, std::string> headerMap = {};
		for (auto pair : luabridge::pairs(headers)) {
			auto key = pair.first.unsafe_cast<std::string>();
			luabridge::LuaRef value = pair.second;
			if (value.type() != LUA_TSTRING) continue;

			auto keyCheck = key;
			std::transform(keyCheck.begin(), keyCheck.end(), keyCheck.begin(), ::toupper);
			if (keyCheck.compare("METHOD") == 0 || keyCheck.compare("USER-AGENT") == 0) continue; // Remove these

			headerMap[key] = value.unsafe_cast<std::string>();
		}
		// ----------

		rawrbox::HTTP::request(
		    url, static_cast<rawrbox::HTTPMethod>(method), headerMap, [callback, L](int code, std::map<std::string, std::string> headerResp, std::string resp) {
			    rawrbox::runOnRenderThread([resp, code, callback, headerResp, L]() {
				    if (code == 0 || (code == 200 && resp.starts_with("Operation timed out after"))) {
					    luabridge::call(callback, false, resp); // curl error
					    return;
				    }

				    auto tbl = luabridge::newTable(L);
				    auto headerTbl = luabridge::newTable(L);

				    for (auto& pair : headerResp) {
					    headerTbl[pair.first] = pair.second;
				    }

				    tbl["status"] = code;
				    tbl["data"] = resp;
				    tbl["headers"] = headerTbl;

				    luabridge::call(callback, true, tbl);
			    });
		    },
		    timeout.value_or(10000));
	}

	void HTTPWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("http")
		    .addFunction("request", &HTTPWrapper::request)
		    .endNamespace();
	}
} // namespace rawrbox
