#include <rawrbox/steamworks/callbacks.hpp>

#include <magic_enum.hpp>

#include <fmt/printf.h>

#include <utility>

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
	// -----------
} // namespace rawrbox
