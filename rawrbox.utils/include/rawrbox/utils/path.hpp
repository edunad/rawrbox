#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

namespace rawrbox {
	class PathUtils {
	public:
		static std::filesystem::path stripRootPath(const std::filesystem::path& path);
		static std::filesystem::path normalizePath(const std::filesystem::path& messyPath);

		static bool isSame(const std::filesystem::path& path1, const std::filesystem::path& path2);
		static bool matchGlob(std::string_view pattern, std::string_view path);

		static std::vector<std::string> glob(const std::filesystem::path& root, bool ignoreFiles = false);
	};
} // namespace rawrbox
