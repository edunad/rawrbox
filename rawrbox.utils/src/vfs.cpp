#include <rawrbox/utils/string.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <algorithm>
#include <bit>
#include <cstring>
#include <map>
#include <mutex>

namespace rawrbox {
	// STREAM ----
	VFSStream::VFSStream(const std::filesystem::path& path) : _stream(path, std::ios::binary | std::ios::ate) {
		if (!this->_stream.is_open()) RAWRBOX_CRITICAL("Failed to open '{}'", path.generic_string());
		this->_size = static_cast<uint64_t>(this->_stream.tellg());
	}

	VFSStream::VFSStream(std::shared_ptr<rawrbox::VPK> vpk, const rawrbox::VPKEntry* entry) : _vpk(std::move(vpk)), _entry(entry), _size(static_cast<uint64_t>(entry->preloadSize) + entry->size) {
		if (!entry->compressed) return;

		this->_data = this->_vpk->read(*entry);
		this->_size = this->_data.size();
	}

	uint64_t VFSStream::size() const { return this->_size; }

	size_t VFSStream::read(uint64_t offset, uint8_t* out, size_t size) {
		if (this->_vpk != nullptr) {
			if (!this->_entry->compressed) return this->_vpk->read(*this->_entry, this->_stream, offset, out, size);
			if (offset >= this->_size) return 0;

			const auto count = static_cast<size_t>(std::min<uint64_t>(size, this->_size - offset));
			std::memcpy(out, this->_data.data() + offset, count);
			return count;
		}

		if (offset >= this->_size) return 0;

		this->_stream.clear();
		this->_stream.seekg(static_cast<std::streamoff>(offset));
		this->_stream.read(std::bit_cast<char*>(out), static_cast<std::streamsize>(std::min<uint64_t>(size, this->_size - offset)));

		return static_cast<size_t>(this->_stream.gcount());
	}
	// -----------

	// PRIVATE ----
	std::vector<rawrbox::VFSMount> VFS::_mounts = {};
	std::shared_mutex VFS::_lock = {};

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> VFS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-VFS");
	// -------------
	// ------------

	std::optional<std::string> VFS::getRelativePath(const std::string& path, const std::string& mountPoint) {
		if (mountPoint.empty()) return path;
		if (path == mountPoint) return "";
		if (path.size() > mountPoint.size() && path.starts_with(mountPoint) && path[mountPoint.size()] == '/') return path.substr(mountPoint.size() + 1);

		return std::nullopt;
	}

	std::pair<std::shared_ptr<rawrbox::VPK>, const rawrbox::VPKEntry*> VFS::find(const std::filesystem::path& path) {
		const auto key = normalize(path);

		for (auto mount = _mounts.rbegin(); mount != _mounts.rend(); ++mount) {
			const auto relative = getRelativePath(key, mount->mountPoint);
			if (!relative.has_value()) continue;

			const auto* entry = mount->vpk->getEntry(*relative);
			if (entry != nullptr) return {mount->vpk, entry};
		}

		return {nullptr, nullptr};
	}

	bool VFS::mount(const std::filesystem::path& path, const std::string& mountPoint) {
		if (!std::filesystem::exists(path)) {
			_logger->warn("VPK '{}' not found!", path.generic_string());
			return false;
		}

		const auto canonical = std::filesystem::weakly_canonical(path);

		std::unique_lock lock(_lock);
		if (std::ranges::any_of(_mounts, [&canonical](const auto& mount) { return mount.path == canonical; })) {
			_logger->warn("VPK '{}' is already mounted", path.generic_string());
			return false;
		}

		auto vpk = std::make_shared<rawrbox::VPK>(path);
		_logger->info("Mounted '{}' on '{}' ({} files)", fmt::styled(path.generic_string(), fmt::fg(fmt::color::coral)), mountPoint, vpk->getEntries().size());

		_mounts.push_back({.path = canonical,
		    .mountPoint = normalize(mountPoint),
		    .vpk = std::move(vpk)});

		return true;
	}

	bool VFS::unmount(const std::filesystem::path& path) {
		const auto canonical = std::filesystem::weakly_canonical(path);

		std::unique_lock lock(_lock);
		return std::erase_if(_mounts, [&canonical](const auto& mount) { return mount.path == canonical; }) > 0;
	}

	std::string VFS::normalize(const std::filesystem::path& path) {
		auto str = path.generic_string();
		std::ranges::replace(str, '\\', '/');

		str = std::filesystem::path(str).lexically_normal().generic_string();
		if (str == ".") return "";
		if (str.ends_with('/')) str.pop_back();

		return rawrbox::StrUtils::toLower(str);
	}

	bool VFS::contains(const std::filesystem::path& path) {
		std::error_code err;
		if (std::filesystem::exists(path, err)) return true;
		const auto key = normalize(path);

		std::shared_lock lock(_lock);
		for (const auto& mount : _mounts) {
			const auto relative = getRelativePath(key, mount.mountPoint);
			if (!relative.has_value()) continue;

			if (mount.vpk->getEntry(*relative) != nullptr || mount.vpk->hasFolder(*relative)) return true;
		}

		return false;
	}

	std::vector<uint8_t> VFS::read(const std::filesystem::path& path) {
		std::error_code err;
		if (std::filesystem::is_regular_file(path, err)) {
			std::ifstream stream(path, std::ios::binary | std::ios::ate);
			if (!stream.is_open()) RAWRBOX_CRITICAL("Failed to read '{}'", path.generic_string());

			std::vector<uint8_t> data(static_cast<size_t>(stream.tellg()));
			stream.seekg(0, std::ios::beg);
			stream.read(std::bit_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));

			return data;
		}

		std::shared_lock lock(_lock);
		auto [vpk, entry] = find(path);
		if (vpk == nullptr) return {};

		return vpk->read(*entry);
	}

	std::unique_ptr<rawrbox::VFSStream> VFS::open(const std::filesystem::path& path) {
		std::error_code err;
		if (std::filesystem::is_regular_file(path, err)) return std::make_unique<rawrbox::VFSStream>(path);

		std::shared_lock lock(_lock);
		auto [vpk, entry] = find(path);
		if (vpk == nullptr) return nullptr;

		return std::make_unique<rawrbox::VFSStream>(std::move(vpk), entry);
	}

	std::vector<std::filesystem::path> VFS::list(const std::filesystem::path& folder, bool recursive) {
		std::map<std::string, std::filesystem::path> files = {};

		// LOOSE ---
		std::error_code err;
		if (std::filesystem::is_directory(folder, err)) {
			if (recursive) {
				for (const auto& entry : std::filesystem::recursive_directory_iterator(folder)) {
					if (entry.is_regular_file()) files.emplace(normalize(entry.path()), entry.path());
				}
			} else {
				for (const auto& entry : std::filesystem::directory_iterator(folder)) {
					if (entry.is_regular_file()) files.emplace(normalize(entry.path()), entry.path());
				}
			}
		}
		// --------

		// PACKED ---
		const auto key = normalize(folder);

		std::shared_lock lock(_lock);
		for (auto mount = _mounts.rbegin(); mount != _mounts.rend(); ++mount) {
			const auto relative = getRelativePath(key, mount->mountPoint);
			if (!relative.has_value()) continue;

			const auto prefix = relative->empty() ? "" : *relative + "/";
			for (const auto& entry : mount->vpk->getEntries()) {
				if (!entry.first.starts_with(prefix)) continue;

				const auto remaining = entry.first.substr(prefix.size());
				if (!recursive && remaining.find('/') != std::string::npos) continue;

				const auto path = folder / remaining;
				files.emplace(normalize(path), path);
			}
		}
		// --------

		std::vector<std::filesystem::path> result = {};
		result.reserve(files.size());

		for (auto& file : files) {
			result.push_back(std::move(file.second));
		}

		return result;
	}
} // namespace rawrbox