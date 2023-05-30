#pragma once

#include <rawrbox/resources/loader.hpp>
#include <rawrbox/utils/crc.hpp>

#include <fmt/format.h>

#include <filesystem>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace rawrbox {
	class RESOURCES {
	protected:
		static std::mutex _threadLock;
		static std::vector<std::unique_ptr<rawrbox::Loader>> _loaders;

		// LOADS ---
		template <class T = rawrbox::Resource>
		static T* getFileImpl(const std::filesystem::path& filePath) {
			auto ext = filePath.extension().generic_string();
			for (auto& loader : _loaders) {
				if (!loader->canLoad(ext)) continue;

				auto ret = loader->getFile<T>(filePath);
				if (ret == nullptr) continue;

				return ret;
			}

			return nullptr;
		}

		template <class T = rawrbox::Resource>
		static T* loadFileImpl(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			// check if it's already loaded
			auto found = getFileImpl<T>(filePath.generic_string());
			if (found != nullptr) return found;

			// load file
			auto ext = filePath.extension().generic_string();
			for (auto& loader : _loaders) {
				if (!loader->canLoad(ext)) continue;

				auto ret = loader->createResource<T>(filePath, loadFlags);
				if (ret == nullptr) continue;

				ret->extention = ext;
				ret->flags = loadFlags;

				// try to see if the file exists to make a crc32 of it
				std::vector<uint8_t> buffer = rawrbox::PathUtils::getRawData(filePath);
				if (buffer.empty()) {
					ret->crc32 = 0;
				} else {
					ret->crc32 = CRC::Calculate(buffer.data(), buffer.size(), CRC::CRC_32());
				}

				ret->status = rawrbox::LoadStatus::LOADING;
				if (!ret->load(buffer)) throw std::runtime_error(fmt::format("[RawrBox-Resources] Failed to load file '{}'", filePath.generic_string()));
				ret->status = rawrbox::LoadStatus::LOADED;

				return ret;
			}

			throw std::runtime_error(fmt::format("[RawrBox-Resources] Attempted to load unknown file extension '{}'. Missing loader!", filePath.generic_string()));
		}
		// ---------

	public:
		static void addLoader(std::unique_ptr<rawrbox::Loader> loader) { _loaders.push_back(std::move(loader)); }

		// ⚠️ NOTE, IT SHOULD BE RAN ON THE MAIN THREAD OF THE APPLICATION, BGFX MIGHT NOT LIKE NON-MAIN THREAD ⚠️
		static void upload() {
			for (auto& loader : _loaders) {
				loader->upload();
			}
		}

		// LOADING ---
		static void loadFolder(const std::filesystem::path& folderPath, std::function<void(std::string)> startLoad = nullptr, std::function<void(std::string)> endLoad = nullptr) {
			for (auto& p : std::filesystem::recursive_directory_iterator(folderPath)) {
				if (!p.is_regular_file()) continue;
				auto file = p.path().generic_string();

				if (startLoad != nullptr) startLoad(file);
				loadFile(p, false);
				if (endLoad != nullptr) endLoad(file);
			}
		}

		template <class T = rawrbox::Resource>
		static T* loadFile(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			const std::lock_guard<std::mutex> mutexGuard(_threadLock);
			return loadFileImpl<T>(filePath, loadFlags);
		}

		template <class T = rawrbox::Resource>
		[[nodiscard]] static T* getFile(const std::filesystem::path& filePath) {
			const std::lock_guard<std::mutex> mutexGuard(_threadLock);
			auto fl = getFileImpl<T>(filePath);
			if (fl == nullptr)
				throw std::runtime_error(fmt::format("[RawrBox-Resources] File '{}' not loaded / found!", filePath.generic_string()));

			return fl;
		}

		static void preLoadFolder(const std::filesystem::path& folderPath) {
			for (auto& p : std::filesystem::recursive_directory_iterator(folderPath)) {
				if (!p.is_regular_file()) continue;
				preLoadFile(p);
			}
		}

		static void preLoadFile(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			auto ext = filePath.extension().generic_string();
			for (auto& loader : _loaders) {
				if (!loader->canLoad(ext)) continue;
				loader->addToPreLoad(filePath, loadFlags);
			}
		}

		static void startPreLoadQueue(std::function<void(std::string, uint32_t)> startLoad = nullptr, std::function<void(std::string, uint32_t)> endLoad = nullptr) {
			for (auto& loader : _loaders) {
				for (auto& file : loader->getPreload()) {
					if (startLoad != nullptr) startLoad(file.first.generic_string(), file.second);
					loadFile(file.first, file.second);
					if (endLoad != nullptr) endLoad(file.first.generic_string(), file.second);
				}
			}
		}

		// -----
		static void shutdown() {
			_loaders.clear();
		}
	};

} // namespace rawrbox
