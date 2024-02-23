#pragma once

#include <rawrbox/scripting/mod.hpp>

namespace rawrbox {
	class ResourcesWrapper {

	public:
		static void preLoadFolder(const std::string& path, lua_State* L);
		static void preLoad(const std::string& path, std::optional<uint32_t> loadFlags, lua_State* L);
		static std::string getContent(const std::optional<std::string>& path, lua_State* L);

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
