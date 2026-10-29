#include <rawrbox/engine/static.hpp>
#include <rawrbox/network/scripting/global/http.hpp>
#include <rawrbox/utils/string.hpp>

namespace rawrbox {
	// PRIVATE -----
	std::unordered_map<uint64_t, luabridge::LuaRef> HTTPGlobal::_callbacks = {};
	uint64_t HTTPGlobal::_id = 0;
	// -------------

	void HTTPGlobal::request(const std::string& url, int method, const luabridge::LuaRef& headers, const luabridge::LuaRef& callback, std::optional<int> timeout) {
		if (url.empty()) throw std::runtime_error("URL cannot be empty");
		if (!headers.isTable()) throw std::runtime_error("Invalid header table");
		if (!callback.isCallable()) throw std::runtime_error("Invalid callback");

		// Setup headers -------
		std::map<std::string, std::string> headerMap = {};
		for (auto pair : luabridge::pairs(headers)) {
			auto key = pair.first.unsafe_cast<std::string>();
			luabridge::LuaRef value = pair.second;
			if (value.type() != LUA_TSTRING) continue;

			auto keyCheck = rawrbox::StrUtils::toUpper(key);
			if (keyCheck == "METHOD" || keyCheck == "USER-AGENT") continue; // Remove these

			headerMap[key] = value.unsafe_cast<std::string>();
		}
		// ----------

		const auto id = ++_id;
		_callbacks.emplace(id, callback);

		rawrbox::HTTP::request(
		    url, static_cast<rawrbox::HTTPMethod>(method), headerMap, [id](int code, const std::map<std::string, std::string>& headerResp, const std::string& resp) {
			    rawrbox::runOnRenderThread([resp, code, headerResp, id]() {
				    auto node = _callbacks.extract(id);
				    if (node.empty()) return; // Mod unloaded

				    const auto& callback = node.mapped();
				    auto* L = callback.state();

				    if (code == 0 || (code == 200 && resp.starts_with("Operation timed out after"))) {
					    auto result = rawrbox::LuaUtils::call(callback, false, resp); // curl error
					    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());

					    return;
				    }

				    auto tbl = luabridge::newTable(L);
				    auto headerTbl = luabridge::newTable(L);

				    for (const auto& pair : headerResp) {
					    headerTbl[pair.first] = pair.second;
				    }

				    tbl["status"] = code;
				    tbl["data"] = resp;
				    tbl["headers"] = headerTbl;

				    auto result = rawrbox::LuaUtils::call(callback, true, tbl);
				    if (result.hasFailed()) fmt::print("Lua error\n  └── {}\n", result.errorMessage());
			    });
		    },
		    timeout.value_or(10000));
	}

	void HTTPGlobal::clear(rawrbox::Mod& mod) {
		std::erase_if(_callbacks, [&mod](const auto& pair) { return lua_mainthread(pair.second.state()) == mod.getEnvironment(); });
	}

	void HTTPGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("http", {})
		    .addFunction("request", &HTTPGlobal::request)
		    .endNamespace();
	}
} // namespace rawrbox
