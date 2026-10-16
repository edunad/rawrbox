#include <rawrbox/utils/md5.hpp>
#include <rawrbox/utils/string.hpp>
#include <rawrbox/utils/vpk.hpp>

#include <zstd.h>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstring>
#include <ranges>

namespace rawrbox {
	VPK::VPK(const std::filesystem::path& path) : _path(path) {
		if (!path.stem().generic_string().ends_with("_dir")) RAWRBOX_CRITICAL("Invalid VPK '{}'! Archive name must end with '_dir.vpk'", path.generic_string());

		// LOAD ---
		std::ifstream stream(path, std::ios::binary | std::ios::ate);
		if (!stream.is_open()) RAWRBOX_CRITICAL("Failed to open VPK '{}'", path.generic_string());

		this->_data.resize(static_cast<size_t>(stream.tellg()));
		stream.seekg(0, std::ios::beg);
		stream.read(std::bit_cast<char*>(this->_data.data()), static_cast<std::streamsize>(this->_data.size()));
		// --------

		if (this->_data.size() < sizeof(rawrbox::VPKHeader)) RAWRBOX_CRITICAL("Invalid VPK '{}'! Missing / Corrupt VPK header data", path.generic_string());

		rawrbox::VPKHeader header = {};
		std::memcpy(&header, this->_data.data(), sizeof(rawrbox::VPKHeader));

		this->validate(header);
		this->parseTree(header);
	}

	void VPK::validate(const rawrbox::VPKHeader& header) {
		if (header.signature != 0x55AA1234) RAWRBOX_CRITICAL("Invalid VPK '{}', bad signature", this->_path.generic_string());
		if (header.version != 2) RAWRBOX_CRITICAL("Invalid VPK '{}', version {} is not supported", this->_path.generic_string(), header.version);

		if (header.otherMD5Size != sizeof(rawrbox::VPKOtherMD5)) RAWRBOX_CRITICAL("Invalid VPK '{}'! Missing checksums", this->_path.generic_string());
		if (header.archiveMD5Size % sizeof(rawrbox::VPKArchiveMD5) != 0) RAWRBOX_CRITICAL("Invalid VPK '{}'! Corrupt archive checksums", this->_path.generic_string());

		const uint64_t archiveStart = sizeof(rawrbox::VPKHeader) + static_cast<uint64_t>(header.treeSize) + header.fileDataSize;
		const uint64_t otherStart = archiveStart + header.archiveMD5Size;
		if (otherStart + header.otherMD5Size + header.signatureSize > this->_data.size()) RAWRBOX_CRITICAL("Invalid VPK '{}'! Files are truncated", this->_path.generic_string());

		rawrbox::VPKOtherMD5 other = {};
		std::memcpy(&other, this->_data.data() + otherStart, sizeof(rawrbox::VPKOtherMD5));

		// CHECKSUMS ---
		if (rawrbox::MD5::calculate(this->_data.data() + sizeof(rawrbox::VPKHeader), header.treeSize) != other.tree) RAWRBOX_CRITICAL("Invalid VPK '{}'! Tree checksum mismatch", this->_path.generic_string());
		if (rawrbox::MD5::calculate(this->_data.data() + archiveStart, header.archiveMD5Size) != other.archiveMD5) RAWRBOX_CRITICAL("Invalid VPK '{}'! Archive checksums mismatch", this->_path.generic_string());
		if (rawrbox::MD5::calculate(this->_data.data(), otherStart + offsetof(rawrbox::VPKOtherMD5, file)) != other.file) RAWRBOX_CRITICAL("Invalid VPK '{}'! File checksum mismatch", this->_path.generic_string());
		// -------------

		// ARCHIVE CHUNKS ---
		for (uint64_t offset = archiveStart; offset < otherStart; offset += sizeof(rawrbox::VPKArchiveMD5)) {
			rawrbox::VPKArchiveMD5 chunk = {};
			std::memcpy(&chunk, this->_data.data() + offset, sizeof(rawrbox::VPKArchiveMD5));

			if (chunk.hashType != 0) RAWRBOX_CRITICAL("Invalid VPK '{}'! Invalid hash type {}", this->_path.generic_string(), chunk.hashType);
			this->_archives[chunk.archiveIndex].chunks.push_back(chunk);
		}

		for (auto& val : this->_archives | std::views::values) {
			std::ranges::sort(val.chunks, {}, &rawrbox::VPKArchiveMD5::offset);
			val.verified.resize(val.chunks.size(), false);
		}
		// -------------
	}

	void VPK::parseTree(const rawrbox::VPKHeader& header) {
		size_t pos = sizeof(rawrbox::VPKHeader);
		const size_t end = pos + header.treeSize;

		this->_fileDataOffset = end;

		const auto readString = [this, &pos, end]() -> std::string {
			const auto* begin = this->_data.data() + pos;
			const auto* terminator = static_cast<const uint8_t*>(std::memchr(begin, 0, end - std::min(pos, end)));
			if (terminator == nullptr) RAWRBOX_CRITICAL("Invalid VPK '{}'! Corrupted file tree", this->_path.generic_string());

			std::string str(std::bit_cast<const char*>(begin), static_cast<size_t>(terminator - begin));
			pos += str.size() + 1;

			return str;
		};

		// Read tree
		while (true) {
			const auto extension = readString();
			if (extension.empty()) break;

			while (true) {
				const auto folder = readString();
				if (folder.empty()) break;

				while (true) {
					const auto name = readString();
					if (name.empty()) break;

					if (pos + sizeof(rawrbox::VPKDirectoryEntry) > end)
						RAWRBOX_CRITICAL("Invalid VPK '{}'! corrupted file tree", this->_path.generic_string());

					rawrbox::VPKDirectoryEntry raw = {};
					std::memcpy(&raw, this->_data.data() + pos, sizeof(rawrbox::VPKDirectoryEntry));
					pos += sizeof(rawrbox::VPKDirectoryEntry);

					if (raw.terminator != 0xFFFF && raw.terminator != 0xFFFE)
						RAWRBOX_CRITICAL("Invalid VPK '{}'! Corrupted file entry", this->_path.generic_string());

					if (pos + raw.preloadBytes > end)
						RAWRBOX_CRITICAL("Invalid VPK '{}'! Corrupted file preload data", this->_path.generic_string());

					if (raw.entryLength > 0) {
						if (raw.archiveIndex == 0x7FFF) {
							if (static_cast<uint64_t>(raw.entryOffset) + raw.entryLength > header.fileDataSize)
								RAWRBOX_CRITICAL("Invalid VPK '{}'! Entry outside of the file data", this->_path.generic_string());
						} else if (!this->_archives.contains(raw.archiveIndex)) {
							RAWRBOX_CRITICAL("Invalid VPK '{}'! Missing checksums for archive {}", this->_path.generic_string(), raw.archiveIndex);
						}
					}

					rawrbox::VPKEntry entry = {};
					entry.crc = raw.crc;
					entry.archive = raw.archiveIndex;
					entry.offset = raw.entryOffset;
					entry.size = raw.entryLength;
					entry.preloadOffset = pos;
					entry.preloadSize = raw.preloadBytes;
					entry.compressed = raw.terminator == 0xFFFE;

					pos += raw.preloadBytes;

					std::string path = folder == " " ? "" : folder + "/";
					if (name != " ") path += name;
					if (extension != " ") path += "." + extension;

					path = rawrbox::StrUtils::toLower(path);
					for (auto slash = path.rfind('/'); slash != std::string::npos; slash = path.rfind('/', slash - 1)) {
						if (!this->_folders.insert(path.substr(0, slash)).second || slash == 0) break;
					}

					this->_entries[path] = entry;
				}
			}
		}
	}

	void VPK::verifyChunks(uint16_t archive, uint64_t offset, uint64_t size) {
		const auto archiveIt = this->_archives.find(archive);
		if (archiveIt == this->_archives.end()) RAWRBOX_CRITICAL("Invalid VPK '{}'! Missing checksums for {}", this->_path.generic_string(), archive);

		auto& vpkArchive = archiveIt->second;
		const uint64_t end = offset + size;

		std::ifstream stream = {};
		std::vector<uint8_t> chunkData = {};

		auto first = std::ranges::upper_bound(vpkArchive.chunks, offset, {}, &rawrbox::VPKArchiveMD5::offset);
		if (first != vpkArchive.chunks.begin()) --first;

		for (auto i = static_cast<size_t>(first - vpkArchive.chunks.begin()); i < vpkArchive.chunks.size(); i++) {
			const auto& chunk = vpkArchive.chunks[i];
			if (static_cast<uint64_t>(chunk.offset) + chunk.size <= offset) continue;
			if (chunk.offset >= end) break;

			// THREAD LOCK ---
			{
				std::scoped_lock lock(this->_verifyLock);
				if (vpkArchive.verified[i]) continue;
			}
			// ---------

			if (!stream.is_open()) {
				stream.open(this->getArchivePath(archive), std::ios::binary);
				if (!stream.is_open())
					RAWRBOX_CRITICAL("Failed to open VPK archive '{}'", this->getArchivePath(archive).generic_string());
			}

			chunkData.resize(chunk.size);
			stream.seekg(static_cast<std::streamoff>(chunk.offset));
			stream.read(std::bit_cast<char*>(chunkData.data()), static_cast<std::streamsize>(chunkData.size()));

			if (stream.gcount() != static_cast<std::streamsize>(chunkData.size()))
				RAWRBOX_CRITICAL("Invalid VPK '{}'! File is truncated", this->getArchivePath(archive).generic_string());

			if (rawrbox::MD5::calculate(chunkData.data(), chunkData.size()) != chunk.md5)
				RAWRBOX_CRITICAL("Invalid VPK '{}'! Checksum mismatch at offset {}", this->getArchivePath(archive).generic_string(), chunk.offset);

			std::scoped_lock lock(this->_verifyLock);
			vpkArchive.verified[i] = true;
		}
	}

	const rawrbox::VPKEntry* VPK::getEntry(const std::string& path) const {
		const auto entry = this->_entries.find(path);
		if (entry == this->_entries.end()) return nullptr;

		return &entry->second;
	}

	bool VPK::hasFolder(const std::string& path) const {
		return path.empty() ? !this->_entries.empty() : this->_folders.contains(path);
	}

	const std::unordered_map<std::string, rawrbox::VPKEntry>& VPK::getEntries() const {
		return this->_entries;
	}

	std::filesystem::path VPK::getArchivePath(uint16_t archive) const {
		return getArchivePath(this->_path, archive);
	}

	std::filesystem::path VPK::getArchivePath(const std::filesystem::path& dirPath, uint16_t archive) {
		const auto stem = dirPath.stem().generic_string();
		if (!stem.ends_with("_dir")) RAWRBOX_CRITICAL("Invalid VPK '{}'! Archive name must end with '_dir.vpk'", dirPath.generic_string());

		return dirPath.parent_path() / fmt::format("{}{:03}.vpk", stem.substr(0, stem.size() - 3), archive); // 000, 001, etc
	}

	std::vector<uint8_t> VPK::read(const rawrbox::VPKEntry& entry) {
		std::vector<uint8_t> data(static_cast<size_t>(entry.preloadSize) + entry.size);
		std::ifstream stream = {};

		this->readStored(entry, stream, 0, data.data(), data.size());
		if (!entry.compressed) return data;

		// ZSTD ---
		const auto size = ZSTD_getFrameContentSize(data.data(), data.size());
		if (size == ZSTD_CONTENTSIZE_UNKNOWN || size == ZSTD_CONTENTSIZE_ERROR) RAWRBOX_CRITICAL("Invalid VPK '{}'! Corrupted compressed entry", this->_path.generic_string());

		std::vector<uint8_t> out(static_cast<size_t>(size));
		const auto written = ZSTD_decompress(out.data(), out.size(), data.data(), data.size());
		if (ZSTD_isError(written) != 0U || written != out.size()) RAWRBOX_CRITICAL("Invalid VPK '{}'! Failed to decompress entry ──> {}", this->_path.generic_string(), ZSTD_getErrorName(written));
		// --------

		return out;
	}

	size_t VPK::read(const rawrbox::VPKEntry& entry, std::ifstream& stream, uint64_t offset, uint8_t* out, size_t size) {
		if (entry.compressed) RAWRBOX_CRITICAL("Invalid VPK '{}'! Compressed entries can only be read whole", this->_path.generic_string());
		return this->readStored(entry, stream, offset, out, size);
	}

	size_t VPK::readStored(const rawrbox::VPKEntry& entry, std::ifstream& stream, uint64_t offset, uint8_t* out, size_t size) {
		const uint64_t total = static_cast<uint64_t>(entry.preloadSize) + entry.size;
		if (offset >= total) return 0;

		size = static_cast<size_t>(std::min<uint64_t>(size, total - offset));
		size_t written = 0;

		// PRELOAD ---
		if (offset < entry.preloadSize) {
			written = std::min<size_t>(size, static_cast<size_t>(entry.preloadSize - offset));
			std::memcpy(out, this->_data.data() + entry.preloadOffset + offset, written);
		}
		// -----------

		if (written == size) return size;

		const uint64_t dataOffset = entry.offset + (offset + written - entry.preloadSize);
		const size_t count = size - written;

		// FILE DATA ---
		if (entry.archive == 0x7FFF) {
			std::memcpy(out + written, this->_data.data() + this->_fileDataOffset + dataOffset, count);
			return size;
		}
		// -----------

		// ARCHIVE ---
		this->verifyChunks(entry.archive, dataOffset, count);

		if (!stream.is_open()) {
			stream.open(this->getArchivePath(entry.archive), std::ios::binary);
			if (!stream.is_open())
				RAWRBOX_CRITICAL("Failed to open VPK '{}'", this->getArchivePath(entry.archive).generic_string());
		}

		stream.clear();
		stream.seekg(static_cast<std::streamoff>(dataOffset));
		stream.read(std::bit_cast<char*>(out + written), static_cast<std::streamsize>(count));

		if (stream.gcount() != static_cast<std::streamsize>(count))
			RAWRBOX_CRITICAL("Invalid VPK '{}'! File is truncated", this->getArchivePath(entry.archive).generic_string());
		// -----------

		return size;
	}
} // namespace rawrbox