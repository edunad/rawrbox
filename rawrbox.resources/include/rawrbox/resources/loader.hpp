#pragma once
#include <rawrbox/resources/resource.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/path.hpp>

#include <fmt/printf.h>

#include <memory>
#include <mutex>
#include <vector>

namespace rawrbox {
	class Loader {
	protected:
		std::mutex _threadLock;
		std::vector<std::unique_ptr<rawrbox::Resource>> _files = {};
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
			this->_logger->info("Content `{}` marked for pre-loading", fmt::styled(path.generic_string(), fmt::fg(fmt::color::coral)));
			this->_preLoadFiles.emplace_back(path, loadFlags);
		}

		[[nodiscard]] virtual bool hasFile(const std::filesystem::path& filePath) const {
			return std::ranges::any_of(this->_files, [&filePath](const auto& file) {
				return rawrbox::PathUtils::isSame(filePath, file->filePath);
			});
		}
		// ----------

		// GET ------
		template <class T>
		T* getFile(const std::filesystem::path& filePath) {
			const std::lock_guard<std::mutex> mutexGuard(_threadLock);
			for (auto& file : this->_files) {
				if (!rawrbox::PathUtils::isSame(filePath, file->filePath)) continue;
				return dynamic_cast<T*>(file.get());
			}

			return nullptr;
		}

		template <class T>
		T* createResource(const std::filesystem::path& filePath, uint32_t /*flags*/ = 0) {
			auto obj = this->createEntry();
			obj->status = rawrbox::LoadStatus::UNLOADED;
			obj->filePath = filePath;

			// store pointer so we can return it
			auto* ptr = obj.get();

			{
				const std::lock_guard<std::mutex> mutexGuard(_threadLock);
				this->_files.push_back(std::move(obj));
			}

			return dynamic_cast<T*>(ptr);
		}
		// -----------

		virtual bool canLoad(const std::string& fileExtention) = 0;
		virtual bool supportsBuffer(const std::string& fileExtention) = 0;

		virtual std::unique_ptr<rawrbox::Resource> createEntry() = 0;
	};
} // namespace rawrbox
