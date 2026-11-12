#pragma once
#include <rawrbox/resources/resource.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <algorithm>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rawrbox {
	struct ResourceSlot {
		rawrbox::Resource* resource = nullptr;
		std::shared_future<void> ready = {};
		bool owner = false;
	};

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
			const auto key = rawrbox::VFS::normalize(path);
			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);

			const auto& queued = std::ranges::find_if(this->_preLoadFiles, [&key](const auto& file) { return rawrbox::VFS::normalize(file.first) == key; });
			if (queued != this->_preLoadFiles.end()) {
				if (queued->second != loadFlags) this->_logger->warn("Content `{}` already marked for pre-loading with flags {}, ignoring flags {}", path.generic_string(), queued->second, loadFlags);
				return;
			}

			this->_logger->debug("Content `{}` marked for pre-loading", fmt::styled(path.generic_string(), fmt::fg(fmt::color::coral)));
			this->_preLoadFiles.emplace_back(path, loadFlags);
		}

		[[nodiscard]] virtual std::vector<std::pair<std::filesystem::path, uint32_t>> processPreload() {
			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);
			return std::exchange(this->_preLoadFiles, {});
		}

		[[nodiscard]] virtual bool hasFile(const std::filesystem::path& filePath) const {
			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);
			return this->_files.contains(rawrbox::VFS::normalize(filePath));
		}
		// ----------

		// GET ------
		template <class T>
		T* getFile(const std::filesystem::path& filePath) {
			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);

			const auto& file = this->_files.find(rawrbox::VFS::normalize(filePath));
			if (file == this->_files.end()) return nullptr;

			return dynamic_cast<T*>(file->second.get());
		}

		[[nodiscard]] rawrbox::ResourceSlot find(const std::filesystem::path& filePath) const {
			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);

			const auto& file = this->_files.find(rawrbox::VFS::normalize(filePath));
			if (file == this->_files.end()) return {};

			return {.resource = file->second.get(), .ready = file->second->ready, .owner = false};
		}

		template <class T>
		[[nodiscard]] rawrbox::ResourceSlot reserve(const std::filesystem::path& filePath, std::promise<void>& promise) {
			const auto key = rawrbox::VFS::normalize(filePath);

			{
				const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);

				const auto& file = this->_files.find(key);
				if (file != this->_files.end()) return {file->second.get(), file->second->ready, false};
			}

			auto obj = this->createEntry();
			if (dynamic_cast<T*>(obj.get()) == nullptr) return {};

			obj->status = rawrbox::LoadStatus::UNLOADED;
			obj->filePath = filePath;
			obj->ready = promise.get_future().share();

			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);
			const auto [file, created] = this->_files.try_emplace(key, std::move(obj));

			return {.resource = file->second.get(), .ready = file->second->ready, .owner = created};
		}

		void release(const std::filesystem::path& filePath) {
			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);
			this->_files.erase(rawrbox::VFS::normalize(filePath));
		}
		// -----------

		virtual bool canLoad(const std::string& fileExtention) = 0;
		virtual bool supportsBuffer(const std::string& fileExtention) = 0;

		virtual std::unique_ptr<rawrbox::Resource> createEntry() = 0;
	};
} // namespace rawrbox