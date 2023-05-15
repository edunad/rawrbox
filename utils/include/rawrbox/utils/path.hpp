#pragma once
#include <filesystem>

namespace rawrbox {
	class PathUtils {
	public:
		static std::filesystem::path stripRootPath(std::filesystem::path path) {
			path = path.relative_path();

			if (path.empty()) return {};
			return path.lexically_relative(*path.begin());
		}

		static std::filesystem::path normalizePath(const std::filesystem::path& messyPath) {
			std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(messyPath);
			return canonicalPath.make_preferred();
		}

		static bool isSame(const std::filesystem::path& path1, const std::filesystem::path& path2) {
			if (path1 == path2) return true;
			return normalizePath(path1) == normalizePath(path2);
		}
	};
} // namespace rawrbox
