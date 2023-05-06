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
	};
} // namespace rawrbox