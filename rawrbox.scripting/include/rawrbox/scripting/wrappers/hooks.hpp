#pragma once
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/logger.hpp>

#include <tracy/Tracy.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
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
		struct StringHash {
			using is_transparent = void;
			[[nodiscard]] size_t operator()(std::string_view id) const { return std::hash<std::string_view>{}(id); }
		};

		using HookList = std::shared_ptr<const std::vector<rawrbox::Hook>>;
		static std::unordered_map<std::string, HookList, StringHash, std::equal_to<>> _hooks;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static int callLua(lua_State* L);

	public:
		template <typename... Args>
		static void call(std::string_view id, const Args&... args) {
			const auto& fnd = Hooks::_hooks.find(id);
			if (fnd == Hooks::_hooks.end()) return;
			ZoneTransientN(zone, fnd->first.c_str(), true);

			const HookList hooks = fnd->second;
			for (const auto& hook : *hooks) {
				ZoneTransientN(hookZone, hook.name.c_str(), true);
				
				auto result = rawrbox::LuaUtils::call(hook.func, args...);
				if (result.hasFailed()) Hooks::_logger->error("Hook '{}' handler '{}' failed\n  └── {}", id, hook.name, result.errorMessage());
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