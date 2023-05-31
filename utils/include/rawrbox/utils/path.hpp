#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

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

		static std::vector<uint8_t> getRawData(const std::filesystem::path& filePath) {
			if (!std::filesystem::exists(filePath)) return {};

			std::vector<uint8_t> file;
			const auto iflags = std::ios::in | std::ios::binary | std::ios::ate;
			if (auto ifs = std::ifstream{filePath.generic_string(), iflags}) {
				file.resize(ifs.tellg());
				ifs.seekg(0, std::ios::beg);
				ifs.read(std::bit_cast<char*>(file.data()), file.size());
			}

			return file;
		}
	};
} // namespace rawrbox
