#include <rawrbox/scripting/wrappers/hooks.hpp>

namespace rawrbox {
	// PRIVATE -----
	std::unordered_map<std::string, std::vector<rawrbox::Hook>> Hooks::_hooks = {};
	// -------------

	void Hooks::call(const std::string& id, const luabridge::LuaRef& args) {
		for (auto& hook : _hooks[id]) {
			luabridge::call(hook.func, args);
		}
	}

	void Hooks::add(const std::string& id, const std::string& name, const luabridge::LuaRef& func) {
		if (!func.isCallable()) throw std::runtime_error("Invalid callback");
		_hooks[id].emplace_back(name, func);
	}

	void Hooks::remove(const std::string& id, const std::string& name) {
		auto nameMap = _hooks.find(id);
		if (nameMap == _hooks.end()) return;

		auto& arr = nameMap->second;
		auto hook = std::find_if(arr.begin(), arr.end(), [&](auto& elem) {
			return elem.name == name;
		});

		if (hook == arr.end()) return;
		arr.erase(hook);

		if (arr.empty()) _hooks.erase(id);
	}

	// Utils ---
	size_t Hooks::count() { return _hooks.size(); }
	bool Hooks::empty() { return _hooks.empty(); }
	// ----

	void Hooks::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("hooks", {})
		    .addFunction("call", &rawrbox::Hooks::call)
		    .addFunction("add", &rawrbox::Hooks::add)
		    .addFunction("remove", &rawrbox::Hooks::remove)
		    .endNamespace();
	}
} // namespace rawrbox
