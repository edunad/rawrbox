#include <rawrbox/steamworks/workshop/storage.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/threading.hpp>

namespace rawrbox {
	// PRIVATE ----------
	std::unique_ptr<rawrbox::Logger> SteamSTORAGE::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SteamSTORAGE");
	rawrbox::Tasker<rawrbox::DownloadRequest> SteamSTORAGE::_downloadQueue = {2};
	std::unordered_map<std::string, std::vector<uint8_t>> SteamSTORAGE::_cache = {};

	bool SteamSTORAGE::isCached(UGCHandle_t handle) {
		return _cache.find(std::to_string(handle)) != _cache.end();
	}

	std::vector<uint8_t> SteamSTORAGE::getCache(UGCHandle_t handle) {
		auto fnd = _cache.find(std::to_string(handle));
		if (fnd == _cache.end()) throw _logger->error("Item {} not cached!", std::to_string(handle));
		return fnd->second;
	}
	// ------------------

	// PUBLIC ------
	void SteamSTORAGE::init() {
		_logger->info("Initializing SteamSTORAGE");
	}

	void SteamSTORAGE::shutdown() {
		_logger.reset();
		_downloadQueue.clear();
		_cache.clear();
	}

	// DOWNLOAD ---
	void SteamSTORAGE::download(UGCHandle_t handle, const std::function<void(std::vector<uint8_t>)>& callback, uint32_t priority) {
		if (SteamRemoteStorage() == nullptr) throw _logger->error("SteamRemoteStorage not initialized");

		// Check cache
		if (isCached(handle)) {
			_logger->info("Using {} UGC cached data", handle);
			callback(getCache(handle));
			return;
		}
		// ---------

		// DOWNLOAD ---
		_downloadQueue.addTask(
		    handle, [callback, handle](const std::vector<uint8_t>& data) {
			    if (!data.empty()) _cache[std::to_string(handle)] = data;
			    callback(data);
		    },
		    priority);
		// -----------
	}
	//-------------
	// -------------
} // namespace rawrbox
