#pragma once

#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/vpk.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Based on :
// https://developer.valvesoftware.com/wiki/VPK_(file_format)
// https://developer.valvesoftware.com/wiki/WAD

namespace rawrbox {
	struct VPKWriterFile {
		std::filesystem::path source = {};
		bool preload = false;
		bool compress = false; // Zstd, only kept if it saves 10%+. Compressed files can't be streamed
	};

	struct VPKWriterTreeFile {
		std::string name = {};
		const rawrbox::VPKWriterFile* file = nullptr;

		rawrbox::VPKDirectoryEntry entry = {};
		std::vector<uint8_t> preload = {};
	};

	class VPKWriter {
	protected:
		std::unordered_map<std::string, rawrbox::VPKWriterFile> _files = {}; // VPK path -> file

		uint64_t _archiveSize = 0;
		uint32_t _chunkSize = 0;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-VPKWriter");
		// -------------

		virtual void writeArchive(const std::filesystem::path& path, uint16_t index, const std::vector<uint8_t>& data, std::vector<rawrbox::VPKArchiveMD5>& chunks);

	public:
		explicit VPKWriter(uint64_t archiveSize = 200ULL * 1024 * 1024, uint32_t chunkSize = 1024 * 1024);
		VPKWriter(const VPKWriter&) = delete;
		VPKWriter(VPKWriter&&) = delete;
		VPKWriter& operator=(const VPKWriter&) = delete;
		VPKWriter& operator=(VPKWriter&&) = delete;
		virtual ~VPKWriter() = default;

		virtual void add(const std::string& path, const std::filesystem::path& source, bool preload = false, bool compress = false);
		virtual void write(const std::filesystem::path& path);
	};
} // namespace rawrbox