#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>

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
			std::vector<uint8_t> data = {};
			if (!std::filesystem::exists(filePath)) return data;

			std::ifstream file(filePath.generic_string(), std::ios::in | std::ios::binary | std::ios::ate);
			if (!file.is_open()) return data;

			auto size = static_cast<size_t>(file.tellg());

			std::vector<char> fileData;
			fileData.resize(size);

			file.seekg(0, std::ios::beg);
			file.read(fileData.data(), size);
			file.close();

			data.insert(data.begin(), fileData.begin(), fileData.end());
			return data;
		}
	};
} // namespace rawrbox
