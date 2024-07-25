#include <rawrbox/steamworks/callbacks/manager.hpp>
#include <rawrbox/steamworks/workshop/storage.hpp>

namespace rawrbox {
	// PRIVATE ----------
	std::unique_ptr<rawrbox::Logger> SteamSTORAGE::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SteamSTORAGE");
	// ------------------

	// PUBLIC ------
	void SteamSTORAGE::init() {
		_logger->info("Initializing SteamSTORAGE");
	}

	void SteamSTORAGE::shutdown() {
		_logger.reset();
	}

	// DOWNLOAD ---
	void SteamSTORAGE::cancelRequests() {
		rawrbox::SteamCALLBACKS::getInstance().cancelAllUGCRequest();
	}

	void SteamSTORAGE::download(UGCHandle_t handle, const std::function<void(std::vector<uint8_t>)>& callback, uint32_t priority) {
		if (SteamRemoteStorage() == nullptr) throw _logger->error("SteamRemoteStorage not initialized");

		// DOWNLOAD ---
		SteamAPICall_t hAPICall = SteamRemoteStorage()->UGCDownload(handle, priority);
		if (hAPICall == k_uAPICallInvalid) return;

		rawrbox::SteamCALLBACKS::getInstance().addUGCRequest(handle, hAPICall, callback);
		// -----------
	}
	//-------------
	// -------------
} // namespace rawrbox
