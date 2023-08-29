#pragma once

#include <rawrbox/scripting/mod.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class ResourcesWrapper {

	public:
		ResourcesWrapper() = default;

		void preLoadFolder(const std::string& path, sol::this_environment modEnv);
		void preLoad(const std::string& path, sol::optional<uint32_t> loadFlags, sol::this_environment modEnv);
		std::string getContent(sol::optional<std::string> path, sol::this_environment modEnv);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
