#include <rawrbox/steamworks/callbacks/manager.hpp>

#include <magic_enum/magic_enum.hpp>

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
	void SteamCALLBACKS::OnWorkshopItemSubscribed(RemoteStoragePublishedFileSubscribed_t* pParam) {
		if (pParam->m_nAppID != STEAMWORKS_APPID) return;

		this->_logger->debug("Steam workshop subscribed: {}", pParam->m_nPublishedFileId);
		this->onModSubscribed(pParam->m_nPublishedFileId);
	}

	void SteamCALLBACKS::OnWorkshopItemInstalled(ItemInstalled_t* pParam) {
		if (pParam->m_unAppID != STEAMWORKS_APPID) return;

		this->_logger->debug("Steam workshop installed: {}", pParam->m_nPublishedFileId);
		this->onModInstalled(pParam->m_nPublishedFileId);
	}

	void SteamCALLBACKS::OnWorkshopItemUnSubscribed(RemoteStoragePublishedFileUnsubscribed_t* pParam) {
		if (pParam->m_nAppID != STEAMWORKS_APPID) return;

		this->_logger->debug("Steam workshop unsubscribed: {}", pParam->m_nPublishedFileId); // Seems to only be triggered after game shutsdown.. wow
		this->onModUnSubscribed(pParam->m_nPublishedFileId);
	}

	void SteamCALLBACKS::OnWorkshopItemDownloaded(DownloadItemResult_t* pParam) {
		if (pParam->m_unAppID != STEAMWORKS_APPID) return;

		this->_logger->debug("Steam workshop updated: {}", pParam->m_nPublishedFileId);
		this->onModUpdated(pParam->m_nPublishedFileId);
	}
	// --------

	// WORKSHOP ---
	void SteamCALLBACKS::OnWorkshopCreateItem(CreateItemResult_t* result, bool bIOFailure) {
		if (bIOFailure) RAWRBOX_CRITICAL("Failed to create workshop item");
		if (this->_CreateItemResultCallback != nullptr) this->_CreateItemResultCallback(result);
	}

	void SteamCALLBACKS::OnWorkshopUpdateItem(SubmitItemUpdateResult_t* result, bool bIOFailure) {
		if (bIOFailure) RAWRBOX_CRITICAL("Failed to update workshop item");
		if (this->_UpdateItemResultCallback != nullptr) this->_UpdateItemResultCallback(result);
	}
	// -------------

	// PUBLIC ---
	SteamCALLBACKS::SteamCALLBACKS() : _IPCFailureCallback(this, &SteamCALLBACKS::OnIPCFailure),
					   _SteamShutdownCallback(this, &SteamCALLBACKS::OnSteamShutdown),
					   _CallbackWorkshopItemInstalled(this, &SteamCALLBACKS::OnWorkshopItemInstalled),
					   _CallbackWorkshopItemUnSubscribed(this, &SteamCALLBACKS::OnWorkshopItemUnSubscribed),
					   _CallbackWorkshopItemSubscribed(this, &SteamCALLBACKS::OnWorkshopItemSubscribed),
					   _CallbackWorkshopItemDownloaded(this, &SteamCALLBACKS::OnWorkshopItemDownloaded) {}

	void SteamCALLBACKS::init() {
		if (this->_initialized) RAWRBOX_CRITICAL("Already initialized");
		this->_initialized = true;
	}

	void SteamCALLBACKS::shutdown() {
		this->_ugcQueries.clear();
		this->_ugcStorageQueries.clear();

		this->_CreateItemResult.Cancel();
		this->_UpdateItemResult.Cancel();

		this->_initialized = false;
	}

	// QUERY ---
	void SteamCALLBACKS::addUGCQueryCallback(SteamAPICall_t apicall, const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback) {
		auto fnd = this->_ugcQueries.find(apicall);
		if (fnd != this->_ugcQueries.end()) RAWRBOX_CRITICAL("AddUGCQueryCallback with api call {} already called! Wait for previous call to complete", apicall);

		std::unique_ptr<rawrbox::SteamUGCQuery> query = std::make_unique<rawrbox::SteamUGCQuery>(apicall, [this, apicall, callback](std::vector<rawrbox::WorkshopMod> details) {
			callback(std::move(details));
			this->_ugcQueries.erase(apicall);
		});

		this->_ugcQueries[apicall] = std::move(query);
	}
	// ----------

	// WORKSHOP ---
	void SteamCALLBACKS::addCreateItemCallback(SteamAPICall_t apicall, const std::function<void(CreateItemResult_t*)>& callback) {
		if (this->_CreateItemResultCallback != nullptr) RAWRBOX_CRITICAL("AddUGCQueryCallback already called! Wait for previous call to complete");

		this->_CreateItemResultCallback = callback;
		this->_CreateItemResult.Set(apicall, this, &SteamCALLBACKS::OnWorkshopCreateItem);
	}

	void SteamCALLBACKS::addUpdateItemCallback(SteamAPICall_t apicall, const std::function<void(SubmitItemUpdateResult_t*)>& callback) {
		if (this->_UpdateItemResultCallback != nullptr) RAWRBOX_CRITICAL("AddUGCQueryCallback already called! Wait for previous call to complete");

		this->_UpdateItemResultCallback = callback;
		this->_UpdateItemResult.Set(apicall, this, &SteamCALLBACKS::OnWorkshopUpdateItem);
	}
	// ----------

	// STORAGE ---
	void SteamCALLBACKS::cancelUGCRequest(SteamAPICall_t handle) {
		auto fnd = this->_ugcStorageQueries.find(handle);
		if (fnd == this->_ugcStorageQueries.end()) return;
		this->_ugcStorageQueries.erase(handle);
	}

	void SteamCALLBACKS::cancelAllUGCRequest() {
		this->_ugcStorageQueries.clear();
	}

	void SteamCALLBACKS::addUGCRequest(UGCHandle_t handle, SteamAPICall_t apicall, const std::function<void(std::vector<uint8_t>)>& callback) {
		auto fnd = this->_ugcStorageQueries.find(apicall);
		if (fnd != this->_ugcStorageQueries.end()) RAWRBOX_CRITICAL("addUGCRequest with api call {} already called! Wait for previous call to complete", apicall);

		std::unique_ptr<rawrbox::SteamStorageRequest> query = std::make_unique<rawrbox::SteamStorageRequest>(handle, apicall, [this, apicall, callback](std::vector<uint8_t> data) {
			callback(std::move(data));
			this->_ugcStorageQueries.erase(apicall);
		});

		this->_ugcStorageQueries[apicall] = std::move(query);
	}
	// -----------
	// -----------
} // namespace rawrbox
