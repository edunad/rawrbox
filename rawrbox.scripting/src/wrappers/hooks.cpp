#include <rawrbox/scripting/wrappers/hooks.hpp>

#include <algorithm>

namespace rawrbox {
	// PRIVATE -----
	std::unordered_map<std::string, std::vector<rawrbox::Hook>> Hooks::_hooks = {};
	// -------------

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> Hooks::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Hooks");
	// -------------

	int Hooks::callLua(lua_State* L) {
		const std::string id = luaL_checkstring(L, 1);

		const auto& fnd = Hooks::_hooks.find(id);
		if (fnd == Hooks::_hooks.end()) return 0;
		ZoneTransientN(zone, id.c_str(), true);

		const int args = lua_gettop(L) - 1;
		const auto hooks = fnd->second;

		for (const auto& hook : hooks) {
			ZoneTransientN(hookZone, hook.name.c_str(), true);

			lua_State* target = hook.func.state();
			const int top = lua_gettop(target);

			hook.func.push(target);

			bool copied = true;
			for (int i = 0; i < args; i++) {
				const auto arg = rawrbox::LuaUtils::copy(L, i + 2, target);
				if (arg) continue;

				Hooks::_logger->error("Hook '{}' handler '{}' skipped\n  └── Argument #{}: {}", id, hook.name, i + 1, arg.error());
				copied = false;
				break;
			}

			if (copied && lua_pcall(target, args, 0, 0) != LUA_OK) Hooks::_logger->error("Hook '{}' handler '{}' failed\n  └── {}", id, hook.name, rawrbox::LuaUtils::getError(target));
			lua_settop(target, top);
		}

		return 0;
	}

	void Hooks::add(const std::string& id, const std::string& name, const luabridge::LuaRef& func) {
		if (!func.isCallable()) throw std::runtime_error("Invalid callback");

		const luabridge::LuaRef handler(lua_mainthread(func.state()), func);

		auto& arr = Hooks::_hooks[id];
		const auto hook = std::ranges::lower_bound(arr, name, {}, &rawrbox::Hook::name);
		if (hook != arr.end() && hook->name == name) {
			hook->func = handler;
			return;
		}

		arr.emplace(hook, name, handler);
	}

	void Hooks::destroy(const std::string& id, const std::string& name) {
		const auto& nameMap = Hooks::_hooks.find(id);
		if (nameMap == Hooks::_hooks.end()) return;

		auto& arr = nameMap->second;
		std::erase_if(arr, [&name](const auto& elem) { return elem.name == name; });
		if (arr.empty()) Hooks::_hooks.erase(nameMap);
	}

	bool Hooks::exists(const std::string& id, const std::string& name) {
		const auto& nameMap = Hooks::_hooks.find(id);
		return nameMap != Hooks::_hooks.end() && std::ranges::any_of(nameMap->second, [&name](const auto& elem) {
			return elem.name == name;
		});
	}

	void Hooks::clear(lua_State* L) {
		if (L == nullptr) {
			Hooks::_hooks.clear();
			return;
		}

		for (auto it = Hooks::_hooks.begin(); it != Hooks::_hooks.end();) {
			std::erase_if(it->second, [L](const auto& elem) { return lua_mainthread(elem.func.state()) == lua_mainthread(L); });
			it = it->second.empty() ? Hooks::_hooks.erase(it) : std::next(it);
		}
	}

	// Utils ---
	size_t Hooks::count() { return Hooks::_hooks.size(); }
	bool Hooks::empty() { return Hooks::_hooks.empty(); }
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