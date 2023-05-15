#pragma once

#include <rawrbox/resources/loader.hpp>
#include <rawrbox/utils/crc.hpp>

#include <fmt/format.h>

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace rawrbox {
	class ResourceManager {
	protected:
		std::mutex _threadLock;
		std::vector<std::unique_ptr<rawrbox::Loader>> _loaders = {};

		// LOADS ---
		std::vector<uint8_t> getRawData(const std::filesystem::path& filePath) {
			std::vector<uint8_t> data = {};
			if (!std::filesystem::exists(filePath)) return data;

			std::ifstream file(filePath.generic_string(), std::ios::in | std::ios::binary | std::ios::ate);
			if (!file.is_open()) return data;

			auto size = static_cast<size_t>(file.tellg());

			std::vector<char> fileData;
			fileData.resize(size);

			file.seekg(0, std::ios::beg);
			file.read(fileData.data(), size);
			file.close();

			data.insert(data.begin(), fileData.begin(), fileData.end());
			return data;
		}

		template <class T = rawrbox::Resource>
		T* getFileImpl(const std::filesystem::path& filePath) {
			auto ext = filePath.extension().generic_string();
			for (auto& loader : this->_loaders) {
				if (!loader->canLoad(ext)) continue;

				auto ret = loader->getFile<T>(filePath);
				if (ret == nullptr) continue;

				return ret;
			}

			return nullptr;
		}

		template <class T = rawrbox::Resource>
		T* loadFileImpl(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			// check if it's already loaded
			auto found = this->getFileImpl<T>(filePath.generic_string());
			if (found != nullptr) return found;

			// load file
			auto ext = filePath.extension().generic_string();
			for (auto& loader : this->_loaders) {
				if (!loader->canLoad(ext)) continue;

				auto ret = loader->createResource<T>(filePath, loadFlags);
				if (ret == nullptr) continue;

				ret->extention = ext;
				ret->flags = loadFlags;

				// try to see if the file exists to make a crc32 of it
				std::vector<uint8_t> buffer = this->getRawData(filePath);
				if (buffer.empty()) {
					ret->crc32 = 0;
				} else {
					ret->crc32 = CRC::Calculate(buffer.data(), buffer.size(), CRC::CRC_32());
				}

				ret->status = rawrbox::LoadStatus::LOADING;
				if (!ret->load(buffer)) throw std::runtime_error(fmt::format("[RawrBox-Content] Failed to load file '{}'", filePath.generic_string()));
				ret->status = rawrbox::LoadStatus::LOADED;

				return ret;
			}

			return nullptr;
		}
		// ---------

	public:
		virtual ~ResourceManager() = default;

		virtual void addLoader(std::unique_ptr<rawrbox::Loader> loader) { this->_loaders.push_back(std::move(loader)); }

		// ⚠️ NOTE, IT SHOULD BE RAN ON THE MAIN THREAD OF THE APPLICATION, BGFX MIGHT NOT LIKE NON-MAIN
		virtual void upload() {
			for (auto& loader : this->_loaders) {
				loader->upload();
			}
		}

		// LOADING ---
		virtual void loadFolder(const std::filesystem::path& folderPath, std::function<void(std::string)> startLoad = nullptr, std::function<void(std::string)> endLoad = nullptr) {
			for (auto& p : std::filesystem::recursive_directory_iterator(folderPath)) {
				if (!p.is_regular_file()) continue;
				auto file = p.path().generic_string();

				if (startLoad != nullptr) startLoad(file);
				this->loadFile(p, false);
				if (endLoad != nullptr) endLoad(file);
			}
		}

		template <class T = rawrbox::Resource>
		T* loadFile(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);
			return loadFileImpl<T>(filePath, loadFlags);
		}

		template <class T = rawrbox::Resource>
		T* getFile(const std::filesystem::path& filePath) {
			const std::lock_guard<std::mutex> mutexGuard(this->_threadLock);
			return getFileImpl<T>(filePath);
		}

		virtual void preLoadFolder(const std::filesystem::path& folderPath) {
			for (auto& p : std::filesystem::recursive_directory_iterator(folderPath)) {
				if (!p.is_regular_file()) continue;
				preLoadFile(p);
			}
		}

		virtual void preLoadFile(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			auto ext = filePath.extension().generic_string();
			for (auto& loader : this->_loaders) {
				if (!loader->canLoad(ext)) continue;
				loader->addToPreLoad(filePath, loadFlags);
			}
		}

		virtual void startPreLoadQueue(std::function<void(std::string, uint32_t)> startLoad = nullptr, std::function<void(std::string, uint32_t)> endLoad = nullptr) {
			for (auto& loader : this->_loaders) {
				for (auto& file : loader->getPreload()) {
					if (startLoad != nullptr) startLoad(file.first.generic_string(), file.second);
					this->loadFile(file.first, file.second);
					if (endLoad != nullptr) endLoad(file.first.generic_string(), file.second);
				}
			}
		}

		// -----
		virtual void shutdown() {
			this->_loaders.clear();
		}
	};

} // namespace rawrbox
