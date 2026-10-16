#pragma once

#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/vpk.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

namespace rawrbox {
	class VFSStream {
	protected:
		std::shared_ptr<rawrbox::VPK> _vpk = nullptr;
		const rawrbox::VPKEntry* _entry = nullptr; // Owned by the VPK

		std::ifstream _stream = {};
		uint64_t _size = 0;

		std::vector<uint8_t> _data = {}; // Compressed entries are decompressed whole

	public:
		explicit VFSStream(const std::filesystem::path& path);
		VFSStream(std::shared_ptr<rawrbox::VPK> vpk, const rawrbox::VPKEntry* entry);
		VFSStream(const VFSStream&) = delete;
		VFSStream(VFSStream&&) = delete;
		VFSStream& operator=(const VFSStream&) = delete;
		VFSStream& operator=(VFSStream&&) = delete;
		virtual ~VFSStream() = default;

		[[nodiscard]] virtual uint64_t size() const;
		virtual size_t read(uint64_t offset, uint8_t* out, size_t size);
	};

	struct VFSMount {
		std::filesystem::path path = {};
		std::string mountPoint = {};
		std::shared_ptr<rawrbox::VPK> vpk = nullptr;
	};

	class VFS {
	protected:
		static std::vector<rawrbox::VFSMount> _mounts;
		static std::shared_mutex _lock;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static std::optional<std::string> getRelativePath(const std::string& path, const std::string& mountPoint);
		static std::pair<std::shared_ptr<rawrbox::VPK>, const rawrbox::VPKEntry*> find(const std::filesystem::path& path);

	public:
		static bool mount(const std::filesystem::path& path, const std::string& mountPoint = "");
		static bool unmount(const std::filesystem::path& path);

		[[nodiscard]] static std::string normalize(const std::filesystem::path& path);
		[[nodiscard]] static bool contains(const std::filesystem::path& path);

		[[nodiscard]] static std::vector<uint8_t> read(const std::filesystem::path& path);
		[[nodiscard]] static std::unique_ptr<rawrbox::VFSStream> open(const std::filesystem::path& path);
		[[nodiscard]] static std::vector<std::filesystem::path> list(const std::filesystem::path& folder, bool recursive = false);
	};
} // namespace rawrbox
