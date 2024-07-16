#include <rawrbox/steamworks/callbacks.hpp>

#include <magic_enum.hpp>

#include <fmt/printf.h>

#include <utility>

namespace rawrbox {
	// PRIVATE ----
	// GLOBAL ---------
	void SteamCALLBACKS::OnIPCFailure(IPCFailure_t* /*failure*/) {
		static bool bExiting = false;

		if (!bExiting) {
			bExiting = true;
			exit(-1); // TODO: shut down game nicely
		}
	}

	void SteamCALLBACKS::OnSteamShutdown(SteamShutdown_t* /*callback*/) {
		static bool bExiting = false;

		if (!bExiting) {
			bExiting = true;
			exit(-1); // TODO: shut down game nicely
		}
	}
	// -------

	// WORKSHOP ----
	void SteamCALLBACKS::OnWorkshopItemInstalled(ItemInstalled_t* pParam) {
		if (pParam->m_unAppID != STEAMWORKS_APPID) return;

		this->_logger->info("Steam workshop installed: {}", pParam->m_nPublishedFileId);
		this->onModInstalled(pParam->m_nPublishedFileId);
	}

	void SteamCALLBACKS::OnWorkshopItemRemoved(RemoteStoragePublishedFileUnsubscribed_t* pParam) {
		if (pParam->m_nAppID != STEAMWORKS_APPID) return;

		this->_logger->info("Steam workshop removed: {}", pParam->m_nPublishedFileId); // Seems to only be triggered after game shutsdown.. wow
		this->onModRemoved(pParam->m_nPublishedFileId);
	}

	void SteamCALLBACKS::OnWorkshopItemDownloaded(DownloadItemResult_t* pParam) {
		if (pParam->m_unAppID != STEAMWORKS_APPID) return;

		this->_logger->info("Steam workshop updated: {}", pParam->m_nPublishedFileId);
		this->onModUpdated(pParam->m_nPublishedFileId);
	}
	// --------

	// WORKSHOP ---
	void SteamCALLBACKS::OnWorkshopCreateItem(CreateItemResult_t* result, bool bIOFailure) {
		if (bIOFailure) throw _logger->error("Failed to create workshop item");
		if (this->_CreateItemResultCallback != nullptr) this->_CreateItemResultCallback(result);
	}

	void SteamCALLBACKS::OnWorkshopUpdateItem(SubmitItemUpdateResult_t* result, bool bIOFailure) {
		if (bIOFailure) throw _logger->error("Failed to update workshop item");
		if (this->_UpdateItemResultCallback != nullptr) this->_UpdateItemResultCallback(result);
	}
	// -------------

	// PUBLIC ---
	SteamCALLBACKS::SteamCALLBACKS() : _IPCFailureCallback(this, &SteamCALLBACKS::OnIPCFailure),
					   _SteamShutdownCallback(this, &SteamCALLBACKS::OnSteamShutdown),
					   _CallbackWorkshopItemInstalled(this, &SteamCALLBACKS::OnWorkshopItemInstalled),
					   _CallbackWorkshopItemRemoved(this, &SteamCALLBACKS::OnWorkshopItemRemoved),
					   _CallbackWorkshopItemDownloaded(this, &SteamCALLBACKS::OnWorkshopItemDownloaded) {}

	void SteamCALLBACKS::init() {
		if (this->_initialized) throw this->_logger->error("Already initialized");

		this->_initialized = true;
		this->_callbackThread = std::make_unique<std::jthread>([this]() {
			while (!this->_callbackShutdown) {
				SteamAPI_RunCallbacks();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		});
	}

	void SteamCALLBACKS::shutdown() {
		this->_ugcQueries.clear();
		this->_ugcStorageQueries.clear();

		this->_CreateItemResult.Cancel();
		this->_UpdateItemResult.Cancel();

		this->_callbackShutdown = true;
		this->_callbackThread->join();
		this->_callbackThread.reset();

		this->_initialized = false;
	}

	// QUERY ---
	void SteamCALLBACKS::addUGCQueryCallback(SteamAPICall_t apicall, const std::function<void(std::vector<SteamUGCDetails_t>)>& callback) {
		auto fnd = this->_ugcQueries.find(apicall);
		if (fnd != this->_ugcQueries.end()) throw _logger->error("AddUGCQueryCallback with api call {} already called! Wait for previous call to complete", apicall);

		std::unique_ptr<rawrbox::SteamUGCQuery> query = std::make_unique<rawrbox::SteamUGCQuery>(apicall, [this, apicall, callback](std::vector<SteamUGCDetails_t> details) {
			callback(std::move(details));
			this->_ugcQueries.erase(apicall);
		});

		this->_ugcQueries[apicall] = std::move(query);
	}
	// ----------

	// WORKSHOP ---
	void SteamCALLBACKS::addCreateItemCallback(SteamAPICall_t apicall, const std::function<void(CreateItemResult_t*)>& callback) {
		if (this->_CreateItemResultCallback != nullptr) throw _logger->error("AddUGCQueryCallback already called! Wait for previous call to complete");

		this->_CreateItemResultCallback = callback;
		this->_CreateItemResult.Set(apicall, this, &SteamCALLBACKS::OnWorkshopCreateItem);
	}

	void SteamCALLBACKS::addUpdateItemCallback(SteamAPICall_t apicall, const std::function<void(SubmitItemUpdateResult_t*)>& callback) {
		if (this->_UpdateItemResultCallback != nullptr) throw _logger->error("AddUGCQueryCallback already called! Wait for previous call to complete");

		this->_UpdateItemResultCallback = callback;
		this->_UpdateItemResult.Set(apicall, this, &SteamCALLBACKS::OnWorkshopUpdateItem);
	}
	// ----------

	// STORAGE ---
	void SteamCALLBACKS::addRequestUGC(UGCHandle_t handle, SteamAPICall_t apicall, const std::function<void(std::vector<uint8_t>)>& callback) {
		auto fnd = this->_ugcStorageQueries.find(apicall);
		if (fnd != this->_ugcStorageQueries.end()) throw _logger->error("AddRequestUGC with api call {} already called! Wait for previous call to complete", apicall);

		std::unique_ptr<rawrbox::SteamStorageRequest> query = std::make_unique<rawrbox::SteamStorageRequest>(handle, apicall, [this, apicall, callback](std::vector<uint8_t> data) {
			callback(std::move(data));
			this->_ugcStorageQueries.erase(apicall);
		});

		this->_ugcStorageQueries[apicall] = std::move(query);
	}
	// -----------
	// -----------
} // namespace rawrbox
