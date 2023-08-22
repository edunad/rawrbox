#pragma once

#include <rawrbox/scripting/mod.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class ResourcesWrapper {
		rawrbox::Mod* _mod = nullptr;

	public:
		ResourcesWrapper(rawrbox::Mod* mod_);

		void preLoadFolder(const std::string& path);
		void preLoad(const std::string& path, sol::optional<uint32_t> loadFlags);
		std::string getContent(sol::optional<std::string> path);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
