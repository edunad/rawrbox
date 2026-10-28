#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

#include <optional>
#include <string>

namespace rawrbox {
	class LoadingGlobal {
	public:
		static void add(const std::string& id, const std::string& title, size_t done, size_t total, const std::optional<std::string>& detail);
		static void remove(const std::string& id);

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox