
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <algorithm>
#include <functional>

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
		if (rawrbox::VFS::normalize(path1) == rawrbox::VFS::normalize(path2)) return true;

		return normalizePath(path1) == normalizePath(path2);
	}

	// Thanks i hate it
	bool PathUtils::matchGlob(std::string_view pattern, std::string_view path) {
		constexpr auto NONE = std::string_view::npos;

		size_t p = 0;
		size_t s = 0;

		size_t starP = NONE;
		size_t starS = 0;

		size_t globP = NONE;
		size_t globS = 0;

		bool globFolder = false;

		while (p < pattern.size() || s < path.size()) {
			if (p < pattern.size()) {
				if (pattern[p] == '*') {
					const auto end = std::min(pattern.find_first_not_of('*', p), pattern.size());
					const bool segment = (p == 0 || pattern[p - 1] == '/') && (end == pattern.size() || pattern[end] == '/');

					if (end - p >= 2 && segment) {
						p = end;

						globFolder = p < pattern.size();
						if (globFolder) p++;

						globP = p;
						globS = s;
						starP = NONE;
						continue;
					}

					p = end;
					starP = p;
					starS = s;
					continue;
				}

				if (s < path.size() && (pattern[p] == '?' ? path[s] != '/' : pattern[p] == path[s])) {
					p++;
					s++;
					continue;
				}
			}

			if (starP != NONE && starS < path.size() && path[starS] != '/') {
				p = starP;
				s = ++starS;
				continue;
			}

			if (globP != NONE && globS < path.size()) {
				if (globFolder) globS = path.find('/', globS);
				if (globS == NONE) return false;

				p = globP;
				s = ++globS;
				starP = NONE;
				continue;
			}

			return false;
		}

		return true;
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