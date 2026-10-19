#pragma once

#include <rawrbox/utils/logger.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rawrbox {
	// https://developer.valvesoftware.com/wiki/VPK_(file_format)
#pragma pack(push, 1)
	struct VPKHeader {
		uint32_t signature = 0x55AA1234;
		uint32_t version = 2;

		uint32_t treeSize = 0;
		uint32_t fileDataSize = 0;
		uint32_t archiveMD5Size = 0;
		uint32_t otherMD5Size = 0;
		uint32_t signatureSize = 0;
	};

	struct VPKDirectoryEntry {
		uint32_t crc = 0;
		uint16_t preloadBytes = 0;
		uint16_t archiveIndex = 0;
		uint32_t entryOffset = 0;
		uint32_t entryLength = 0;
		uint16_t terminator = 0xFFFF; // 0xFFFE = Zstd compressed (rawrbox only)
	};

	struct VPKArchiveMD5 {
		uint16_t archiveIndex = 0;
		uint16_t hashType = 0;
		uint32_t offset = 0;
		uint32_t size = 0;
		std::array<uint8_t, 16> md5 = {};
	};

	struct VPKOtherMD5 {
		std::array<uint8_t, 16> tree = {};
		std::array<uint8_t, 16> archiveMD5 = {};
		std::array<uint8_t, 16> file = {};
	};
#pragma pack(pop)

	struct VPKEntry {
		uint32_t crc = 0;
		uint16_t archive = 0;
		uint32_t offset = 0;
		uint32_t size = 0;

		size_t preloadOffset = 0;
		uint16_t preloadSize = 0;

		bool compressed = false;
	};

	struct VPKArchive {
		std::vector<rawrbox::VPKArchiveMD5> chunks = {};
		std::vector<bool> verified = {};
	};

	class VPK {
	protected:
		std::filesystem::path _path = {};

		std::vector<uint8_t> _data = {};
		size_t _fileDataOffset = 0;

		std::unordered_map<std::string, rawrbox::VPKEntry> _entries = {};
		std::unordered_set<std::string> _folders = {};
		std::unordered_map<uint16_t, rawrbox::VPKArchive> _archives = {};
		std::mutex _verifyLock;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-VPK");
		// -------------

		void validate(const rawrbox::VPKHeader& header);
		void parseTree(const rawrbox::VPKHeader& header);
		virtual void verifyChunks(uint16_t archive, uint64_t offset, uint64_t size);
		virtual size_t readStored(const rawrbox::VPKEntry& entry, std::ifstream& stream, uint64_t offset, uint8_t* out, size_t size);

	public:
		explicit VPK(const std::filesystem::path& path);
		VPK(const VPK&) = delete;
		VPK(VPK&&) = delete;
		VPK& operator=(const VPK&) = delete;
		VPK& operator=(VPK&&) = delete;
		virtual ~VPK() = default;

		[[nodiscard]] virtual const rawrbox::VPKEntry* getEntry(const std::string& path) const;
		[[nodiscard]] virtual bool hasFolder(const std::string& path) const;
		[[nodiscard]] virtual const std::unordered_map<std::string, rawrbox::VPKEntry>& getEntries() const;
		[[nodiscard]] virtual std::filesystem::path getArchivePath(uint16_t archive) const;

		// ex: pak01_dir.vpk + 1 -> pak01_001.vpk
		[[nodiscard]] static std::filesystem::path getArchivePath(const std::filesystem::path& dirPath, uint16_t archive);

		[[nodiscard]] virtual std::vector<uint8_t> read(const rawrbox::VPKEntry& entry);
		virtual size_t read(const rawrbox::VPKEntry& entry, std::ifstream& stream, uint64_t offset, uint8_t* out, size_t size);
	};
} // namespace rawrbox
