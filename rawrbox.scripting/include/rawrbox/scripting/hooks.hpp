#pragma once
#include <rawrbox/scripting/utils/lua.hpp>

#include <string>
#include <unordered_map>

namespace rawrbox {
	struct Hook {
		std::string name;
		luabridge::LuaRef func;

		Hook(std::string name_, luabridge::LuaRef func_) : name(std::move(name_)), func(std::move(func_)) {}
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

		void listen(const std::string& id, const std::string& name, const luabridge::LuaRef& func);
		void remove(const std::string& id, const std::string& name);

		// Utils ---
		[[nodiscard]] size_t count() const;
		[[nodiscard]] bool empty() const;
		// ----
	};
} // namespace rawrbox
