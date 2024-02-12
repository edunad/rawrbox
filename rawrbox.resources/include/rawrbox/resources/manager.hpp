#pragma once

#include <rawrbox/resources/loader.hpp>
#include <rawrbox/utils/crc.hpp>
#include <rawrbox/utils/threading.hpp>

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
		static std::vector<std::filesystem::path> _loadedFiles;
		static std::vector<std::unique_ptr<rawrbox::Loader>> _loaders;

		static std::atomic<size_t> _loadingFiles;
		static std::atomic<size_t> _loadingPreloadFiles;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

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
			std::string path = filePath.generic_string();

			// check if it's already loaded
			auto found = getFileImpl<T>(path);
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
				std::vector<uint8_t> buffer = {};

				if (loader->supportsBuffer(ext)) {
					buffer = rawrbox::PathUtils::getRawData(filePath);

					if (buffer.empty()) {
						ret->crc32 = 0;
					} else {
						ret->crc32 = CRC::Calculate(buffer.data(), buffer.size(), CRC::CRC_32());
					}
				}

				ret->status = rawrbox::LoadStatus::LOADING;

				if (!ret->load(buffer)) throw _logger->error("Failed to load file '{}'", path);
				ret->upload();

				ret->status = rawrbox::LoadStatus::LOADED;

				// Add to the list for later on easy access
				{
					const std::lock_guard<std::mutex> mutexGuard(_threadLock);
					_loadedFiles.push_back(filePath);
				}
				// ---

				return ret;
			}

			throw _logger->error("Attempted to load unknown file extension '{}'. Missing loader!", filePath.generic_string());
		}
		// ---------

	public:
		template <class T, typename... CallbackArgs>
		static void addLoader(CallbackArgs&&... args) {
			_loaders.push_back(std::make_unique<T>(std::forward<CallbackArgs>(args)...));
		}

		static const std::vector<std::unique_ptr<rawrbox::Loader>>& getLoaders() { return _loaders; }

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

		static void loadFolderAsync(const std::filesystem::path& folderPath, std::function<void(std::string)> startLoad = nullptr, std::function<void(std::string)> endLoad = nullptr) {
			for (auto& p : std::filesystem::recursive_directory_iterator(folderPath)) {
				if (!p.is_regular_file()) continue;
				auto file = p.path().generic_string();

				rawrbox::ASYNC::run([startLoad, endLoad, file, p]() {
					if (startLoad != nullptr) startLoad(file);
					loadFile(p, false);
					if (endLoad != nullptr) endLoad(file);
				});
			}
		}

		template <class T = rawrbox::Resource>
		static T* loadFile(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			if (filePath.empty()) throw _logger->error("Attempted to load empty path");
			return loadFileImpl<T>(filePath, loadFlags);
		}

		template <class T = rawrbox::Resource>
		static void loadListAsync(const std::vector<std::pair<std::string, uint32_t>>& files, std::function<void()> onComplete = nullptr) {
			_loadingFiles += files.size();

			std::function<void()> complete = [onComplete]() {
				_loadingFiles = std::max<size_t>(_loadingFiles - 1, 0);
				if (_loadingFiles <= 0 && onComplete != nullptr) onComplete();
			};

			for (auto& file : files) {
				rawrbox::ASYNC::run([file, &complete]() {
					loadFileImpl<T>(file.first, file.second);
					_logger->info("Loaded '{}'", fmt::format(fmt::fg(fmt::color::coral), file.first));

					complete();
				});
			}
		}

		template <class T = rawrbox::Resource>
		static void loadFileAsync(const std::filesystem::path& filePath, uint32_t loadFlags = 0, std::function<void()> onComplete = nullptr) {
			if (filePath.empty()) throw _logger->error("Attempted to load empty path");

			rawrbox::ASYNC::run([filePath, loadFlags, onComplete]() {
				loadFileImpl<T>(filePath, loadFlags);
				_logger->info("Loaded '{}'", fmt::format(fmt::fg(fmt::color::coral), filePath.generic_string()));

				if (onComplete != nullptr) onComplete();
			});
		}

		template <class T = rawrbox::Resource>
		[[nodiscard]] static T* getFile(const std::filesystem::path& filePath) {
			auto fl = getFileImpl<T>(filePath);
			if (fl == nullptr) {
				throw _logger->error("File '{}' not loaded / found!", filePath.generic_string());
			}

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

		static void startPreLoadQueueAsync(std::function<void(std::string, uint32_t)> startLoad = nullptr, std::function<void(std::string, uint32_t)> endLoad = nullptr, std::function<void()> onComplete = nullptr) {
			_loadingPreloadFiles += getTotalPreload();

			std::function<void()> complete = [onComplete]() {
				_loadingPreloadFiles = std::max<size_t>(_loadingPreloadFiles - 1, 0);
				if (_loadingPreloadFiles <= 0 && onComplete != nullptr) onComplete();
			};

			for (auto& loader : _loaders) {
				for (auto& file : loader->getPreload()) {
					rawrbox::ASYNC::run([startLoad, &file, endLoad, &complete]() {
						if (startLoad != nullptr) startLoad(file.first.generic_string(), file.second);
						loadFile(file.first, file.second);
						if (endLoad != nullptr) endLoad(file.first.generic_string(), file.second);

						_logger->info("Loaded '{}'", fmt::format(fmt::fg(fmt::color::coral), file.first.generic_string()));
						complete();
					});
				}
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

		static size_t getTotalPreload() {
			size_t total = 0;
			for (auto& loader : _loaders) {
				total += loader->getPreload().size();
			}

			return total;
		}

		static size_t filesLoaded() {
			return _loadedFiles.size();
		}

		static bool isLoaded(const std::filesystem::path& filePath) {
			return std::find(_loadedFiles.begin(), _loadedFiles.end(), filePath) != _loadedFiles.end();
		}

		// -----
		static void shutdown() {
			_loaders.clear();
			_logger.reset();
		}
	};

} // namespace rawrbox
