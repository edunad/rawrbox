
#include <rawrbox/utils/path.hpp>

#include <bit>
#include <fstream>
#include <functional>
#include <iostream>

namespace rawrbox {
	std::filesystem::path PathUtils::stripRootPath(const std::filesystem::path& path) {
		auto r = path.relative_path();

		if (r.empty()) return {};
		return r.lexically_relative(*r.begin());
	}

	std::filesystem::path PathUtils::normalizePath(const std::filesystem::path& messyPath) {
		std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(messyPath);
		return canonicalPath.make_preferred();
	}

	bool PathUtils::isSame(const std::filesystem::path& path1, const std::filesystem::path& path2) {
		if (path1 == path2) return true;
		return normalizePath(path1) == normalizePath(path2);
	}

	std::vector<uint8_t> PathUtils::getRawData(const std::filesystem::path& filePath) {
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

	std::vector<std::string> PathUtils::glob(const std::filesystem::path& root, bool ignoreFiles) {
		std::vector<std::string> dirs = {};

		std::function<void(const std::filesystem::path& path)> globSearch;
		globSearch = [&globSearch, &dirs, ignoreFiles](const std::filesystem::path& path) -> void {
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				if (!ignoreFiles) dirs.push_back(entry.path().generic_string());

				if (std::filesystem::is_directory(entry)) {
					if (ignoreFiles) dirs.push_back(entry.path().generic_string());
					globSearch(entry);
				}
			}
		};

		globSearch(root);
		return dirs;
	}

} // namespace rawrbox
