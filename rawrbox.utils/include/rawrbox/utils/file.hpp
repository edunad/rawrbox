#pragma once

#include <bit>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace rawrbox {
	class FileUtils {
	public:
		template <typename T = uint8_t>
		static std::vector<T> getRawData(const std::filesystem::path& filePath) {
			if (!std::filesystem::exists(filePath)) return {};

			std::vector<T> file = {};
			const auto iflags = std::ios::in | std::ios::binary | std::ios::ate;
			if (auto ifs = std::ifstream(filePath.generic_string(), iflags)) {
				const auto fileSize = ifs.tellg();

				if (fileSize > 0 && fileSize <= std::numeric_limits<std::streamsize>::max()) {
					file.resize(static_cast<size_t>(fileSize));
					ifs.seekg(0, std::ios::beg);
					ifs.read(std::bit_cast<char*>(file.data()), static_cast<std::streamsize>(file.size()));
				}
			}

			file.push_back('\0');
			return file;
		}

		template <typename T = uint8_t>
		static bool saveData(const std::filesystem::path& filePath, const std::vector<T>& data) {
			if (data.empty()) return false;

			const auto oflags = std::ios::out | std::ios::binary;
			if (auto ofs = std::ofstream(filePath.generic_string(), oflags)) {
				ofs.write(std::bit_cast<const char*>(data.data()), data.size());
				return true;
			}

			return false;
		}
	};
} // namespace rawrbox
