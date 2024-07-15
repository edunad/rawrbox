#include <rawrbox/steamworks/callbacks.hpp>
#include <rawrbox/steamworks/workshop/storage.hpp>

#include <filesystem>

namespace rawrbox {
	// PRIVATE ----------
	std::unique_ptr<rawrbox::Logger> SteamSTORAGE::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SteamSTORAGE");
	// ------------------

	// PUBLIC ------
	void SteamSTORAGE::init() {
		_logger->info("Initializing SteamSTORAGE");

		if (!std::filesystem::exists(".cache")) {
			std::filesystem::create_directory(".cache");
		}
	}

	// DOWNLOAD ---
	void SteamSTORAGE::download(UGCHandle_t handle, const std::function<void(std::vector<uint8_t>)>& callback, uint32_t priority) {
		if (SteamRemoteStorage() == nullptr) throw _logger->error("SteamRemoteStorage not initialized");

		SteamAPICall_t hAPICall = SteamRemoteStorage()->UGCDownload(handle, priority);
		if (hAPICall == k_uAPICallInvalid) throw _logger->error("Failed to download UGC handle {}", handle);

		rawrbox::SteamCALLBACKS::getInstance().addRequestUGC(handle, hAPICall, callback); // TODO: QUEUE THE REQUEST INSTEAD, THIS LAGS THE THREAD
	}
	//-------------
	// -------------
} // namespace rawrbox
