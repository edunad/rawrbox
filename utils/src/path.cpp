
#include <rawrbox/utils/path.hpp>

#include <bit>
#include <fstream>
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

} // namespace rawrbox
