#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class IOWrapper {

	public:
		static std::pair<std::string, std::string> load(const std::string& path);
		static std::string save(const std::string& path, const std::string& data);

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
