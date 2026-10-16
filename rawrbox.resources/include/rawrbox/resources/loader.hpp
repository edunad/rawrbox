#pragma once
#include <rawrbox/resources/resource.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace rawrbox {
	class Loader {
	protected:
		mutable std::mutex _threadLock;

		std::unordered_map<std::string, std::unique_ptr<rawrbox::Resource>> _files = {};
		std::vector<std::pair<std::filesystem::path, uint32_t>> _preLoadFiles = {};

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Loader");
		// -------------

	public:
		Loader() = default;
		Loader(const Loader&) = delete;
		Loader(Loader&&) = delete;
		Loader& operator=(const Loader&) = delete;
		Loader& operator=(Loader&&) = delete;
		virtual ~Loader() = default;

		// UTILS -----
		[[nodiscard]] virtual const std::vector<std::pair<std::filesystem::path, uint32_t>>& getPreload() const { return this->_preLoadFiles; }
		virtual void addToPreLoad(const std::filesystem::path& path, uint32_t loadFlags = 0) {
			this->_logger->debug("Content `{}` marked for pre-loading", fmt::styled(path.generic_string(), fmt::fg(fmt::color::coral)));
			this->_preLoadFiles.emplace_back(path, loadFlags);
		}

		[[nodiscard]] virtual bool hasFile(const std::filesystem::path& filePath) const {
			const std::lock_guard<std::mutex> mutexGuard(_threadLock);
			return this->_files.contains(rawrbox::VFS::normalize(filePath));
		}
		// ----------

		// GET ------
		template <class T>
		T* getFile(const std::filesystem::path& filePath) {
			const std::lock_guard<std::mutex> mutexGuard(_threadLock);

			const auto file = this->_files.find(rawrbox::VFS::normalize(filePath));
			if (file == this->_files.end()) return nullptr;

			return dynamic_cast<T*>(file->second.get());
		}

		template <class T>
		T* createResource(const std::filesystem::path& filePath, uint32_t /*flags*/ = 0) {
			auto obj = this->createEntry();
			obj->status = rawrbox::LoadStatus::UNLOADED;
			obj->filePath = filePath;

			auto* ptr = dynamic_cast<T*>(obj.get());
			if (ptr == nullptr) return nullptr;

			const std::lock_guard<std::mutex> mutexGuard(_threadLock);
			if (!this->_files.try_emplace(rawrbox::VFS::normalize(filePath), std::move(obj)).second) return nullptr;

			return ptr;
		}
		// -----------

		virtual bool canLoad(const std::string& fileExtention) = 0;
		virtual bool supportsBuffer(const std::string& fileExtention) = 0;

		virtual std::unique_ptr<rawrbox::Resource> createEntry() = 0;
	};
} // namespace rawrbox