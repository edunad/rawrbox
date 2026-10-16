#include <rawrbox/utils/crc.hpp>
#include <rawrbox/utils/file.hpp>
#include <rawrbox/utils/md5.hpp>
#include <rawrbox/utils/vfs.hpp>
#include <rawrbox/utils/vpk_writer.hpp>

#include <zstd.h>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <fstream>
#include <limits>
#include <map>
#include <ranges>

namespace rawrbox {
	VPKWriter::VPKWriter(uint64_t archiveSize, uint32_t chunkSize) : _archiveSize(archiveSize), _chunkSize(chunkSize) {
		if (this->_archiveSize == 0 || this->_archiveSize > std::numeric_limits<uint32_t>::max())
			RAWRBOX_CRITICAL("Archive size must be between 1 byte and 4 GB");

		if (this->_chunkSize == 0)
			RAWRBOX_CRITICAL("Chunk size cannot be 0");
	}

	void VPKWriter::add(const std::string& path, const std::filesystem::path& source, bool preload, bool compress) {
		auto vpkPath = rawrbox::VFS::normalize(path);
		if (vpkPath.empty() || vpkPath == ".." || vpkPath.starts_with("/") || vpkPath.starts_with("../") || std::filesystem::path(vpkPath).has_root_name()) RAWRBOX_CRITICAL("Invalid VPK path '{}', it must be relative", path);
		if (vpkPath.ends_with('.')) RAWRBOX_CRITICAL("Invalid VPK path '{}', it cannot end with a '.'", path);
		if (!this->_files.try_emplace(vpkPath, rawrbox::VPKWriterFile{source, preload, compress}).second) RAWRBOX_CRITICAL("Duplicated VPK path '{}'", vpkPath);
	}

	void VPKWriter::write(const std::filesystem::path& path) {
		if (!path.stem().generic_string().ends_with("_dir")) RAWRBOX_CRITICAL("Invalid VPK '{}', expected a '_dir.vpk' file", path.generic_string());

		const auto folder = path.parent_path();
		if (!folder.empty()) std::filesystem::create_directories(folder);

		std::map<std::string, std::map<std::string, std::vector<rawrbox::VPKWriterTreeFile>>> tree = {};

		// SORT (extension -> folder -> name) ---
		for (const auto& [vpkPath, vpkFile] : this->_files) {
			const std::filesystem::path filePath = vpkPath;

			auto extension = filePath.extension().generic_string();
			extension = extension.empty() ? " " : extension.substr(1);

			auto fileFolder = filePath.parent_path().generic_string();
			if (fileFolder.empty()) fileFolder = " ";

			auto name = filePath.stem().generic_string();
			if (name.empty()) name = " ";

			tree[extension][fileFolder].push_back({name, &vpkFile});
		}

		for (auto& folders : tree | std::views::values) {
			for (auto& files : folders | std::views::values) {
				std::ranges::sort(files, {}, &rawrbox::VPKWriterTreeFile::name);
			}
		}
		// -------------

		// DATA ---
		std::vector<uint8_t> fileData = {};
		std::vector<uint8_t> archive = {};

		std::vector<rawrbox::VPKArchiveMD5> chunks = {};
		uint16_t archiveIndex = 0;

		std::unordered_map<std::string, rawrbox::VPKDirectoryEntry> written = {}; // Stored data md5 -> entry, for duplicated files
		size_t compressed = 0;
		size_t duplicated = 0;

		for (auto& extension : tree) {
			for (auto& fileFolder : extension.second) {
				for (auto& file : fileFolder.second) {
					const auto& vpkFile = *file.file;

					std::ifstream stream(vpkFile.source, std::ios::binary | std::ios::ate);
					if (!stream.is_open()) RAWRBOX_CRITICAL("Failed to open '{}'", vpkFile.source.generic_string());

					std::vector<uint8_t> data(static_cast<size_t>(stream.tellg()));
					stream.seekg(0, std::ios::beg);
					stream.read(std::bit_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));

					if (data.size() > std::numeric_limits<uint32_t>::max()) RAWRBOX_CRITICAL("File '{}' is too big for a VPK", vpkFile.source.generic_string());

					auto& entry = file.entry;
					entry.crc = CRC::Calculate(data.data(), data.size(), CRC::CRC_32());

					// COMPRESS ---
					if (vpkFile.compress && !data.empty()) {
						std::vector<uint8_t> packed(ZSTD_compressBound(data.size()));

						const auto size = ZSTD_compress(packed.data(), packed.size(), data.data(), data.size(), 9);
						if (ZSTD_isError(size) != 0U) RAWRBOX_CRITICAL("Failed to compress '{}' ──> {}", vpkFile.source.generic_string(), ZSTD_getErrorName(size));

						if (size <= data.size() - (data.size() / 10)) {
							packed.resize(size);
							data = std::move(packed);

							entry.terminator = 0xFFFE;
							compressed++;
						}
					}
					// -----------

					// DUPLICATED ---
					const bool inlined = vpkFile.preload && data.size() <= std::numeric_limits<uint16_t>::max();
					const auto key = fmt::format("{}|{}", vpkFile.preload, rawrbox::MD5::toString(rawrbox::MD5::calculate(data.data(), data.size())));

					auto found = written.find(key);
					if (!inlined && found != written.end()) {
						entry.archiveIndex = found->second.archiveIndex;
						entry.entryOffset = found->second.entryOffset;
						entry.entryLength = found->second.entryLength;

						duplicated++;
						continue;
					}
					// -----------

					if (inlined) {
						entry.archiveIndex = 0x7FFF;
						entry.preloadBytes = static_cast<uint16_t>(data.size());
						file.preload = std::move(data);
					} else if (vpkFile.preload) {
						entry.archiveIndex = 0x7FFF;
						entry.entryOffset = static_cast<uint32_t>(fileData.size());
						entry.entryLength = static_cast<uint32_t>(data.size());
						fileData.insert(fileData.end(), data.begin(), data.end());
					} else {
						if (!archive.empty() && archive.size() + data.size() > this->_archiveSize) {
							this->writeArchive(rawrbox::VPK::getArchivePath(path, archiveIndex), archiveIndex, archive, chunks);
							archive.clear();

							archiveIndex++;
							if (archiveIndex >= 0x7FFF) RAWRBOX_CRITICAL("Too many VPK archives");
						}

						entry.archiveIndex = archiveIndex;
						entry.entryOffset = static_cast<uint32_t>(archive.size());
						entry.entryLength = static_cast<uint32_t>(data.size());
						archive.insert(archive.end(), data.begin(), data.end());
					}

					if (!inlined) written.emplace(key, entry);
				}
			}
		}

		uint16_t totalArchives = archiveIndex;
		if (!archive.empty()) {
			this->writeArchive(rawrbox::VPK::getArchivePath(path, archiveIndex), archiveIndex, archive, chunks);
			totalArchives++;
		}
		// -------------

		// BUILD TREE ---
		std::vector<uint8_t> treeData = {};
		for (const auto& extension : tree) {
			treeData.insert(treeData.end(), extension.first.c_str(), extension.first.c_str() + extension.first.size() + 1);

			for (const auto& fileFolder : extension.second) {
				treeData.insert(treeData.end(), fileFolder.first.c_str(), fileFolder.first.c_str() + fileFolder.first.size() + 1);

				for (const auto& file : fileFolder.second) {
					treeData.insert(treeData.end(), file.name.c_str(), file.name.c_str() + file.name.size() + 1);
					treeData.insert(treeData.end(), std::bit_cast<const uint8_t*>(&file.entry), std::bit_cast<const uint8_t*>(&file.entry) + sizeof(rawrbox::VPKDirectoryEntry));
					treeData.insert(treeData.end(), file.preload.begin(), file.preload.end());
				}

				treeData.push_back(0);
			}

			treeData.push_back(0);
		}

		treeData.push_back(0);
		// -------------

		// DIRECTORY ---
		if (treeData.size() > std::numeric_limits<uint32_t>::max() || fileData.size() > std::numeric_limits<uint32_t>::max())
			RAWRBOX_CRITICAL("VPK '{}' preload directory is too big!", path.generic_string());

		rawrbox::VPKHeader header = {};
		header.treeSize = static_cast<uint32_t>(treeData.size());
		header.fileDataSize = static_cast<uint32_t>(fileData.size());
		header.archiveMD5Size = static_cast<uint32_t>(chunks.size() * sizeof(rawrbox::VPKArchiveMD5));
		header.otherMD5Size = sizeof(rawrbox::VPKOtherMD5);

		rawrbox::VPKOtherMD5 other = {};
		other.tree = rawrbox::MD5::calculate(treeData.data(), treeData.size());
		other.archiveMD5 = rawrbox::MD5::calculate(std::bit_cast<const uint8_t*>(chunks.data()), header.archiveMD5Size);

		std::vector<uint8_t> dir = {};
		dir.insert(dir.end(), std::bit_cast<const uint8_t*>(&header), std::bit_cast<const uint8_t*>(&header) + sizeof(rawrbox::VPKHeader));
		dir.insert(dir.end(), treeData.begin(), treeData.end());
		dir.insert(dir.end(), fileData.begin(), fileData.end());
		dir.insert(dir.end(), std::bit_cast<const uint8_t*>(chunks.data()), std::bit_cast<const uint8_t*>(chunks.data()) + header.archiveMD5Size);
		dir.insert(dir.end(), std::bit_cast<const uint8_t*>(&other), std::bit_cast<const uint8_t*>(&other) + offsetof(rawrbox::VPKOtherMD5, file));

		other.file = rawrbox::MD5::calculate(dir.data(), dir.size());
		dir.insert(dir.end(), other.file.begin(), other.file.end());

		if (!rawrbox::FileUtils::saveData(path, dir)) RAWRBOX_CRITICAL("Failed to write VPK '{}'", path.generic_string());
		// -------------

		// STALE ARCHIVES ---
		for (uint16_t i = totalArchives; i < 0x7FFF; i++) {
			const auto stale = rawrbox::VPK::getArchivePath(path, i);
			if (!std::filesystem::exists(stale)) break;

			std::filesystem::remove(stale);
		}
		// -------------

		this->_logger->info("Packed {} files into '{}' ({} archives, {} compressed, {} duplicated)", this->_files.size(), path.generic_string(), totalArchives, compressed, duplicated);
	}

	void VPKWriter::writeArchive(const std::filesystem::path& path, uint16_t index, const std::vector<uint8_t>& data, std::vector<rawrbox::VPKArchiveMD5>& chunks) {
		// GENERATE CHECKSUMS ---
		for (uint64_t offset = 0; offset < data.size(); offset += this->_chunkSize) {
			rawrbox::VPKArchiveMD5 chunk = {};

			chunk.archiveIndex = index;
			chunk.offset = static_cast<uint32_t>(offset);
			chunk.size = static_cast<uint32_t>(std::min<uint64_t>(this->_chunkSize, data.size() - offset));
			chunk.md5 = rawrbox::MD5::calculate(data.data() + offset, chunk.size);

			chunks.push_back(chunk);
		}
		// -------------

		// SKIP UNCHANGED ---
		if (std::filesystem::exists(path) && std::filesystem::file_size(path) == data.size()) {
			std::ifstream stream(path, std::ios::binary);
			std::vector<uint8_t> existing(data.size());
			stream.read(std::bit_cast<char*>(existing.data()), static_cast<std::streamsize>(existing.size()));

			if (existing == data) {
				this->_logger->debug("Archive '{}' is unchanged", path.generic_string());
				return;
			}
		}
		// -------------

		if (!rawrbox::FileUtils::saveData(path, data)) RAWRBOX_CRITICAL("Failed to write VPK archive '{}'!", path.generic_string());
	}
} // namespace rawrbox