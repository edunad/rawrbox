#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

#include <sol/sol.hpp>

#include <fmt/printf.h>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rawrbox {
	struct Hook {
		std::string name;
		sol::function func;

		Hook(std::string name_, sol::function func_) : name(std::move(name_)), func(std::move(func_)) {}
	};

	class Hooks {
		std::unordered_map<std::string, std::vector<Hook>> hooks;

	public:
		template <typename... CallbackArgs>
		void call(const std::string& name, CallbackArgs... args) {
			for (auto& hook : hooks[name]) {
				rawrbox::LuaUtils::runCallback(hook.func, std::forward<CallbackArgs>(args)...);
			}
		}

		void listen(const std::string& id, const std::string& name, sol::function func) {
			hooks[id].emplace_back(name, func);
		}

		void remove(const std::string& id, const std::string& name) {
			auto nameMap = hooks.find(id);
			if (nameMap == hooks.end()) return;

			auto& arr = nameMap->second;
			auto hook = std::find_if(arr.begin(), arr.end(), [&](auto& elem) {
				return elem.name == name;
			});
			if (hook == arr.end()) return;

			arr.erase(hook);
		}
	};
} // namespace rawrbox
