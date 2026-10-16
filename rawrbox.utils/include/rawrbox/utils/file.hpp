#pragma once

#include <rawrbox/utils/vfs.hpp>

#include <bit>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <type_traits>
#include <vector>

namespace rawrbox {
	class FileUtils {
	public:
		template <typename T = uint8_t>
		static std::vector<T> getRawData(const std::filesystem::path& filePath) {
			if (!rawrbox::VFS::contains(filePath)) return {};

			auto data = rawrbox::VFS::read(filePath);

			if constexpr (std::is_same_v<T, uint8_t>) {
				data.push_back('\0'); // Add null terminator
				return data;
			} else {
				std::vector<T> file((data.size() / sizeof(T)) + 1);
				std::memcpy(file.data(), data.data(), (data.size() / sizeof(T)) * sizeof(T));
				return file;
			}
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