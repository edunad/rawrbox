#include <rawrbox/scripting/wrappers/hooks.hpp>

#include <algorithm>

namespace rawrbox {
	// PRIVATE -----
	std::unordered_map<std::string, std::vector<rawrbox::Hook>> Hooks::_hooks = {};
	// -------------

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> Hooks::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Hooks");
	// -------------

	void Hooks::callLua(const std::string& id, const luabridge::LuaRef& args) {
		const auto fnd = _hooks.find(id);
		if (fnd == _hooks.end()) return;
		e const auto data = rawrbox::LuaUtils::luaToJsonObject(args);
		const auto hooks = fnd->second;

		for (const auto& hook : hooks) {
			auto result = rawrbox::LuaUtils::call(hook.func, rawrbox::LuaUtils::jsonToLua(hook.func.state(), data));
			if (result.hasFailed()) _logger->error("Hook '{}' handler '{}' failed\n  └── {}", id, hook.name, result.errorMessage());
		}
	}

	void Hooks::add(const std::string& id, const std::string& name, const luabridge::LuaRef& func) {
		if (!func.isCallable()) throw std::runtime_error("Invalid callback");

		auto& arr = _hooks[id];
		const auto hook = std::ranges::lower_bound(arr, name, {}, &rawrbox::Hook::name);
		if (hook != arr.end() && hook->name == name) {
			hook->func = func;
			return;
		}

		arr.emplace(hook, name, func);
	}

	void Hooks::destroy(const std::string& id, const std::string& name) {
		const auto nameMap = _hooks.find(id);
		if (nameMap == _hooks.end()) return;

		auto& arr = nameMap->second;
		std::erase_if(arr, [&name](const auto& elem) { return elem.name == name; });
		if (arr.empty()) _hooks.erase(nameMap);
	}

	bool Hooks::exists(const std::string& id, const std::string& name) {
		const auto nameMap = _hooks.find(id);
		return nameMap != _hooks.end() && std::ranges::any_of(nameMap->second, [&name](const auto& elem) {
			return elem.name == name;
		});
	}

	void Hooks::clear(lua_State* L) {
		if (L == nullptr) {
			_hooks.clear();
			return;
		}

		for (auto it = _hooks.begin(); it != _hooks.end();) {
			std::erase_if(it->second, [L](const auto& elem) { return lua_mainthread(elem.func.state()) == lua_mainthread(L); });
			it = it->second.empty() ? _hooks.erase(it) : std::next(it);
		}
	}

	// Utils ---
	size_t Hooks::count() { return _hooks.size(); }
	bool Hooks::empty() { return _hooks.empty(); }
	// ----

	void Hooks::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("hooks", {})
		    .addFunction("call", &rawrbox::Hooks::callLua)
		    .addFunction("add", &rawrbox::Hooks::add)
		    .addFunction("destroy", &rawrbox::Hooks::destroy)
		    .addFunction("exists", &rawrbox::Hooks::exists)
		    .endNamespace();
	}
} // namespace rawrbox