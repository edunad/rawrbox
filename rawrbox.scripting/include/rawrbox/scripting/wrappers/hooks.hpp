#pragma once
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/logger.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rawrbox {
	struct Hook {
		std::string name;
		luabridge::LuaRef func;

		Hook(std::string name_, luabridge::LuaRef func_) : name(std::move(name_)), func(std::move(func_)) {}
	};

	class Hooks {
	private:
		static std::unordered_map<std::string, std::vector<rawrbox::Hook>> _hooks;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static void callLua(const std::string& id, const luabridge::LuaRef& args);

	public:
		template <typename... Args>
		static void call(const std::string& id, const Args&... args) {
			const auto fnd = _hooks.find(id);
			if (fnd == _hooks.end()) return;

			const auto hooks = fnd->second;
			for (const auto& hook : hooks) {
				auto result = rawrbox::LuaUtils::call(hook.func, args...);
				if (result.hasFailed()) _logger->error("Hook '{}' handler '{}' failed\n  └── {}", id, hook.name, result.errorMessage());
			}
		}

		static void add(const std::string& id, const std::string& name, const luabridge::LuaRef& func);
		static void destroy(const std::string& id, const std::string& name);
		[[nodiscard]] static bool exists(const std::string& id, const std::string& name);

		static void clear(lua_State* L = nullptr);

		// Utils ---
		[[nodiscard]] static size_t count();
		[[nodiscard]] static bool empty();
		// ----

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox