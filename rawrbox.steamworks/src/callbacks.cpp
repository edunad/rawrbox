#include <rawrbox/steamworks/callbacks.hpp>

#include <magic_enum.hpp>

#include <fmt/printf.h>

namespace rawrbox {
	SteamCALLBACKS::SteamCALLBACKS() : _IPCFailureCallback(this, &SteamCALLBACKS::OnIPCFailure),
					   _SteamShutdownCallback(this, &SteamCALLBACKS::OnSteamShutdown),
					   _CallbackWorkshopItemInstalled(this, &SteamCALLBACKS::OnWorkshopItemInstalled),
					   _CallbackWorkshopItemRemoved(this, &SteamCALLBACKS::OnWorkshopItemRemoved),
					   _CallbackWorkshopItemDownloaded(this, &SteamCALLBACKS::OnWorkshopItemDownloaded) {}

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

	// QUERY ---
	void SteamCALLBACKS::OnUGCQueryCompleted(SteamUGCQueryCompleted_t* pParam, bool bIOFailure) {
		std::vector<SteamUGCDetails_t> details = {};

		if (bIOFailure || pParam->m_eResult != k_EResultOK || pParam->m_unNumResultsReturned <= 0) {
			this->_logger->warn("Failed to query workshop mods: '{}'", magic_enum::enum_name(pParam->m_eResult));
		} else {
			for (uint32_t i = 0; i < pParam->m_unNumResultsReturned; i++) {
				SteamUGCDetails_t detail = {};

				if (SteamUGC()->GetQueryUGCResult(pParam->m_handle, i, &detail)) {
					if (detail.m_eResult == k_EResultOK && detail.m_eFileType == k_EWorkshopFileTypeCommunity) {
						details.push_back(detail);
					}
				}
			}
		}

		if (this->_UGCQueryCompletedCallback != nullptr) this->_UGCQueryCompletedCallback(details);
		SteamUGC()->ReleaseQueryUGCRequest(pParam->m_handle);
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
	// QUERY ---
	void SteamCALLBACKS::addUGCQueryCallback(SteamAPICall_t apicall, const std::function<void(std::vector<SteamUGCDetails_t>)>& callback) {
		if (this->_UGCQueryCompletedCallback != nullptr) throw _logger->error("AddUGCQueryCallback already called! Wait for previous call to complete");

		this->_UGCQueryCompletedCallback = callback;
		this->_steamUGCQueryCompletedResult.Set(apicall, this, &SteamCALLBACKS::OnUGCQueryCompleted);
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
	// -----------
} // namespace rawrbox
