#pragma once
#include <rawrbox/resources/resource.hpp>
#include <rawrbox/utils/path.hpp>

#include <fmt/printf.h>

#include <memory>
#include <vector>

namespace rawrbox {
	class Loader {
	protected:
		std::mutex _threadLock;
		std::vector<std::unique_ptr<rawrbox::Resource>> _files = {};
		std::vector<std::pair<std::filesystem::path, uint32_t>> _preLoadFiles = {};

	public:
		Loader() = default;
		virtual ~Loader() = default;

		// UTILS -----
		[[nodiscard]] virtual const std::vector<std::pair<std::filesystem::path, uint32_t>>& getPreload() const { return this->_preLoadFiles; }
		virtual void addToPreLoad(const std::filesystem::path& path, uint32_t loadFlags = 0) {
			fmt::print("[RawrBox-Resources] Content `{}` marked for pre-loading\n", path.generic_string());
			this->_preLoadFiles.emplace_back(path, loadFlags);
		}

		[[nodiscard]] virtual const bool hasFile(const std::filesystem::path& filePath) const {
			for (auto& file : this->_files) {
				std::error_code err;
				if (!rawrbox::PathUtils::isSame(filePath, file->filePath)) continue;

				return true;
			}

			return false;
		}
		// ----------

		// GET ------
		template <class T>
		T* getFile(const std::filesystem::path& filePath) {
			const std::lock_guard<std::mutex> mutexGuard(_threadLock);
			for (auto& file : this->_files) {
				std::error_code err;
				if (!rawrbox::PathUtils::isSame(filePath, file->filePath)) continue;

				return dynamic_cast<T*>(file.get());
			}

			return nullptr;
		}

		template <class T>
		T* createResource(const std::filesystem::path& filePath, uint32_t flags = 0) {
			auto obj = this->createEntry(flags);
			obj->status = rawrbox::LoadStatus::UNLOADED;
			obj->filePath = filePath;

			// store pointer so we can return it
			auto ptr = obj.get();

			{
				const std::lock_guard<std::mutex> mutexGuard(_threadLock);
				this->_files.push_back(std::move(obj));
			}

			return dynamic_cast<T*>(ptr);
		}
		// -----------

		virtual bool canLoad(const std::string& fileExtention) = 0;
		virtual std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) = 0;
	};
} // namespace rawrbox
