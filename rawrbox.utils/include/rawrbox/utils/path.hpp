#pragma once

#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace rawrbox {
	class PathUtils {
	public:
		static std::filesystem::path stripRootPath(const std::filesystem::path& path);
		static std::filesystem::path normalizePath(const std::filesystem::path& messyPath);
		static bool isSame(const std::filesystem::path& path1, const std::filesystem::path& path2);

		template <typename T = uint8_t>
		static std::vector<T> getRawData(const std::filesystem::path& filePath) {
			if (!std::filesystem::exists(filePath)) return {};

			std::vector<T> file = {};
			const auto iflags = std::ios::in | std::ios::binary | std::ios::ate;
			if (auto ifs = std::ifstream(filePath.generic_string(), iflags)) {
				file.resize(ifs.tellg());
				ifs.seekg(0, std::ios::beg);
				ifs.read(std::bit_cast<char*>(file.data()), file.size());
			}

			file.push_back('\0');
			return file;
		}

		static std::vector<std::string> glob(const std::filesystem::path& root, bool ignoreFiles = false);
	};
} // namespace rawrbox
