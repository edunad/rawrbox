#pragma once

#include <rawrbox/resources/loader.hpp>
#include <rawrbox/utils/crc.hpp>
#include <rawrbox/utils/file.hpp>
#include <rawrbox/utils/threading.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <tracy/Tracy.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace rawrbox {
	struct PreloadProgress {
		size_t loaded = 0;
		size_t total = 0;
	};

	class RESOURCES {
	protected:
		static std::mutex _threadLock;
		static std::unordered_set<std::string> _loadedFiles;
		static std::vector<std::unique_ptr<rawrbox::Loader>> _loaders;

		// PRELOAD ---
		static std::mutex _preloadLock;
		static std::atomic<size_t> _preloadLoaded;
		static std::atomic<size_t> _preloadTotal;
		// -----------

		static thread_local std::vector<std::string> _loading;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		// LOADS ---
		static void waitLoaded(const rawrbox::ResourceSlot& slot, const std::string& key) {
			if (!slot.ready.valid()) return;

			if (slot.ready.wait_for(std::chrono::seconds(0)) != std::future_status::ready && std::ranges::contains(RESOURCES::_loading, key)) {
				RAWRBOX_CRITICAL("Recursive load of '{}'", slot.resource->filePath.generic_string());
			}

			slot.ready.get();
		}

		template <class T = rawrbox::Resource>
			requires(std::derived_from<T, rawrbox::Resource>)
		static T* getFileImpl(const std::filesystem::path& filePath) {
			const auto key = rawrbox::VFS::normalize(filePath);
			const auto ext = filePath.extension().generic_string();

			for (const auto& loader : RESOURCES::_loaders) {
				if (!loader->canLoad(ext)) continue;

				const auto found = loader->find(filePath);
				if (found.resource == nullptr) continue;

				RESOURCES::waitLoaded(found, key);

				auto* ret = dynamic_cast<T*>(found.resource);
				if (ret == nullptr) continue;

				return ret;
			}

			return nullptr;
		}

		template <class T = rawrbox::Resource>
			requires(std::derived_from<T, rawrbox::Resource>)
		static T* loadFileImpl(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			const std::string path = filePath.generic_string();
			const auto key = rawrbox::VFS::normalize(filePath);
			const auto ext = filePath.extension().generic_string();

			for (const auto& loader : RESOURCES::_loaders) {
				if (!loader->canLoad(ext)) continue;

				std::promise<void> promise;
				const auto slot = loader->reserve<T>(filePath, promise);
				if (slot.resource == nullptr) continue;

				if (!slot.owner) {
					RESOURCES::waitLoaded(slot, key);

					auto* found = dynamic_cast<T*>(slot.resource);
					if (found == nullptr) continue;

					return found;
				}

				ZoneScopedN("RESOURCES::loadFile");
				ZoneText(path.c_str(), path.size());

				auto* ret = dynamic_cast<T*>(slot.resource);
				RESOURCES::_loading.push_back(key);

				try {
					ret->extention = ext;
					ret->flags = loadFlags;

					std::vector<uint8_t> buffer = {};
					if (loader->supportsBuffer(ext)) {
						buffer = rawrbox::FileUtils::getRawData(filePath);
						if (buffer.empty()) throw std::runtime_error("File not found or empty");

						ret->crc32 = CRC::Calculate(buffer.data(), buffer.size(), CRC::CRC_32());
					}

					ret->status = rawrbox::LoadStatus::LOADING;
					if (!ret->load(buffer)) throw std::runtime_error("Loader rejected the file");

					ret->upload();
					ret->status = rawrbox::LoadStatus::LOADED;
				} catch (const std::exception& e) {
					std::erase(RESOURCES::_loading, key);

					const std::string error = fmt::format("Failed to load file '{}'\n  └── {}", path, e.what());
					promise.set_exception(std::make_exception_ptr(std::runtime_error(error)));

					loader->release(filePath);
					RAWRBOX_CRITICAL("{}", error);
				}

				std::erase(RESOURCES::_loading, key);

				// Add to the list for later on easy access
				{
					const std::lock_guard<std::mutex> mutexGuard(RESOURCES::_threadLock);
					RESOURCES::_loadedFiles.insert(key);
				}
				// ---

				promise.set_value();
				return ret;
			}

			RAWRBOX_CRITICAL("Attempted to load unknown file extension '{}'. Missing loader!", filePath.generic_string());
		}

		static std::vector<std::pair<std::filesystem::path, uint32_t>> processPreloadQueue() {
			std::vector<std::pair<std::filesystem::path, uint32_t>> queue = {};
			std::unordered_set<std::string> queued = {};

			for (const auto& loader : RESOURCES::_loaders) {
				for (auto& file : loader->processPreload()) {
					if (!queued.insert(rawrbox::VFS::normalize(file.first)).second) continue;
					queue.push_back(std::move(file));
				}
			}

			const std::lock_guard<std::mutex> mutexGuard(RESOURCES::_preloadLock);
			if (RESOURCES::_preloadLoaded.load() >= RESOURCES::_preloadTotal.load()) {
				RESOURCES::_preloadLoaded = 0;
				RESOURCES::_preloadTotal = 0;
			}

			RESOURCES::_preloadTotal += queue.size();
			return queue;
		}
		// ---------

	public:
		template <class T, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::Loader>)
		static void addLoader(CallbackArgs&&... args) {
			RESOURCES::_loaders.push_back(std::make_unique<T>(std::forward<CallbackArgs>(args)...));
		}

		static const std::vector<std::unique_ptr<rawrbox::Loader>>& getLoaders() { return RESOURCES::_loaders; }

		// LOADING ---
		static void loadFolder(const std::filesystem::path& folderPath, const std::function<void(std::string)>& startLoad = nullptr, const std::function<void(std::string)>& endLoad = nullptr) {
			for (const auto& p : rawrbox::VFS::list(folderPath, true)) {
				const auto file = p.generic_string();

				if (startLoad != nullptr) startLoad(file);
				RESOURCES::loadFile(p, 0U);
				if (endLoad != nullptr) endLoad(file);
			}
		}

		static void loadFolderAsync(const std::filesystem::path& folderPath, const std::function<void(std::string)>& startLoad = nullptr, const std::function<void(std::string)>& endLoad = nullptr) {
			for (const auto& p : rawrbox::VFS::list(folderPath, true)) {
				const auto file = p.generic_string();

				rawrbox::ASYNC::run([startLoad, endLoad, file, p]() {
					if (startLoad != nullptr) startLoad(file);
					RESOURCES::loadFile(p, 0U);
					if (endLoad != nullptr) endLoad(file);
				});
			}
		}

		template <class T = rawrbox::Resource>
			requires(std::derived_from<T, rawrbox::Resource>)
		static T* loadFile(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			if (filePath.empty()) RAWRBOX_CRITICAL("Attempted to load empty path");
			return RESOURCES::loadFileImpl<T>(filePath, loadFlags);
		}

		template <class T = rawrbox::Resource>
			requires(std::derived_from<T, rawrbox::Resource>)
		static void loadListAsync(const std::vector<std::pair<std::string, uint32_t>>& files, const std::function<void()>& onComplete = nullptr) {
			if (files.empty()) {
				if (onComplete != nullptr) onComplete();
				return;
			}

			auto remaining = std::make_shared<std::atomic<size_t>>(files.size());
			for (const auto& file : files) {
				rawrbox::ASYNC::run([file, onComplete, remaining]() {
					loadFileImpl<T>(file.first, file.second);
					RESOURCES::_logger->debug("Loaded '{}'", fmt::styled(file.first, fmt::fg(fmt::color::coral)));

					if (remaining->fetch_sub(1) == 1 && onComplete != nullptr) onComplete();
				});
			}
		}

		template <class T = rawrbox::Resource>
			requires(std::derived_from<T, rawrbox::Resource>)
		static void loadFileAsync(const std::filesystem::path& filePath, uint32_t loadFlags = 0, const std::function<void()>& onComplete = nullptr) {
			if (filePath.empty()) RAWRBOX_CRITICAL("Attempted to load empty path");

			rawrbox::ASYNC::run([filePath, loadFlags, onComplete]() {
				loadFileImpl<T>(filePath, loadFlags);
				RESOURCES::_logger->debug("Loaded '{}'", fmt::format(fmt::fg(fmt::color::coral), filePath.generic_string()));

				if (onComplete != nullptr) onComplete();
			});
		}

		template <class T = rawrbox::Resource>
			requires(std::derived_from<T, rawrbox::Resource>)
		[[nodiscard]] static T* getFile(const std::filesystem::path& filePath) {
			auto fl = RESOURCES::getFileImpl<T>(filePath);
			if (fl == nullptr) {
				RAWRBOX_CRITICAL("File '{}' not loaded / found!", filePath.generic_string());
			}

			return fl;
		}

		static void preLoadFolder(const std::filesystem::path& folderPath) {
			for (const auto& p : rawrbox::VFS::list(folderPath, true)) {
				RESOURCES::preLoadFile(p);
			}
		}

		static void preLoadFile(const std::filesystem::path& filePath, uint32_t loadFlags = 0) {
			auto ext = filePath.extension().generic_string();
			for (auto& loader : RESOURCES::_loaders) {
				if (!loader->canLoad(ext)) continue;
				loader->addToPreLoad(filePath, loadFlags);
			}
		}

		static void startPreLoadQueueAsync(const std::function<void(std::string, uint32_t)>& startLoad = nullptr, const std::function<void(std::string, uint32_t)>& endLoad = nullptr, const std::function<void()>& onComplete = nullptr) {
			auto queue = RESOURCES::processPreloadQueue();
			if (queue.empty()) {
				if (onComplete != nullptr) onComplete();
				return;
			}

			auto remaining = std::make_shared<std::atomic<size_t>>(queue.size());
			for (auto& file : queue) {
				rawrbox::ASYNC::run([startLoad, file = std::move(file), endLoad, onComplete, remaining]() {
					if (startLoad != nullptr) startLoad(file.first.generic_string(), file.second);
					RESOURCES::loadFile(file.first, file.second);
					if (endLoad != nullptr) endLoad(file.first.generic_string(), file.second);

					RESOURCES::_logger->debug("Loaded '{}'", fmt::styled(file.first.generic_string(), fmt::fg(fmt::color::coral)));

					++RESOURCES::_preloadLoaded;
					if (remaining->fetch_sub(1) == 1 && onComplete != nullptr) onComplete();
				});
			}
		}

		static void startPreLoadQueue(const std::function<void(std::string, uint32_t)>& startLoad = nullptr, const std::function<void(std::string, uint32_t)>& endLoad = nullptr) {
			for (const auto& file : RESOURCES::processPreloadQueue()) {
				if (startLoad != nullptr) startLoad(file.first.generic_string(), file.second);
				RESOURCES::loadFile(file.first, file.second);
				if (endLoad != nullptr) endLoad(file.first.generic_string(), file.second);

				++RESOURCES::_preloadLoaded;
			}
		}

		static size_t getTotalPreload() {
			std::unordered_set<std::string> queued = {};
			for (const auto& loader : RESOURCES::_loaders) {
				for (const auto& file : loader->getPreload()) {
					queued.insert(rawrbox::VFS::normalize(file.first));
				}
			}

			return queued.size();
		}

		[[nodiscard]] static rawrbox::PreloadProgress getPreloadProgress() {
			return {.loaded = RESOURCES::_preloadLoaded.load(), .total = RESOURCES::_preloadTotal.load()};
		}

		[[nodiscard]] static bool isPreloading() {
			return RESOURCES::_preloadLoaded.load() < RESOURCES::_preloadTotal.load();
		}

		static size_t filesLoaded() {
			const std::lock_guard<std::mutex> mutexGuard(RESOURCES::_threadLock);
			return RESOURCES::_loadedFiles.size();
		}

		static bool isLoaded(const std::filesystem::path& filePath) {
			const std::lock_guard<std::mutex> mutexGuard(RESOURCES::_threadLock);
			return RESOURCES::_loadedFiles.contains(rawrbox::VFS::normalize(filePath));
		}

		// -----
		static void shutdown() {
			RESOURCES::_loaders.clear();
			RESOURCES::_logger.reset();
		}
	};

} // namespace rawrbox