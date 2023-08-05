#pragma once

#include <rawrbox/scripting/utils/lua.hpp>

#include <filesystem>

namespace rawrbox {

	// @/ == Root content
	// @cats/ == `cats` mod
	// normal_path == current mod
	std::string LuaUtils::getContent(const std::string& path, const std::string& modName) {
		if (path.empty()) return fmt::format("{}", modName);
		if (path.starts_with("mods/")) return path; // Already has the mod

		std::string fixedPath = path;
		std::replace(fixedPath.begin(), fixedPath.end(), '\\', '/'); // Fix windows

		// content/blabalba.png = my current mod
		if (!modName.empty() && fixedPath.front() != '@') {
			return std::filesystem::path(fmt::format("{}/{}", modName, fixedPath)).string(); // Becomes mods/mymod/content/blabalba.png
		} else if (fixedPath.front() == '@') {
			auto slashPos = fixedPath.find("/"); // Find the first /
			std::string cleanPath = fixedPath.substr(slashPos + 1);

			// @/textures/blabalba.png = c++ content
			if (fixedPath.rfind("@/", 0) == 0) { // C++
				return std::filesystem::path(fmt::format("content/{}", cleanPath)).string();
			} else { // @otherMod/textures/blabalba.png = @othermod content
				return std::filesystem::path(fmt::format("{}/{}", fixedPath.substr(1, slashPos - 1), cleanPath)).string();
			}
		}

		return path;
	}

} // namespace rawrbox
