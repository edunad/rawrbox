#pragma once

#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/logger.hpp>

#include <steam/steam_api.h>

#include <functional>
#include <memory>
#include <vector>

namespace rawrbox {
	class SteamCALLBACKS {
	protected:
		// QUERY ----
		std::function<void(std::vector<SteamUGCDetails_t>)> _UGCQueryCompletedCallback = nullptr;
		CCallResult<rawrbox::SteamCALLBACKS, SteamUGCQueryCompleted_t> _steamUGCQueryCompletedResult = {};
		// --------

		// WORKSHOP ----
		std::function<void(CreateItemResult_t*)> _CreateItemResultCallback = nullptr;
		std::function<void(SubmitItemUpdateResult_t*)> _UpdateItemResultCallback = nullptr;

		CCallResult<rawrbox::SteamCALLBACKS, CreateItemResult_t> _CreateItemResult;
		CCallResult<rawrbox::SteamCALLBACKS, SubmitItemUpdateResult_t> _UpdateItemResult;
		// ---------

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-SteamCALLBACKS");
		// -------------

#pragma warning(push)
#pragma warning(disable : 4068)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
		// Global --
		STEAM_CALLBACK(rawrbox::SteamCALLBACKS, OnIPCFailure, IPCFailure_t, _IPCFailureCallback);
		STEAM_CALLBACK(rawrbox::SteamCALLBACKS, OnSteamShutdown, SteamShutdown_t, _SteamShutdownCallback);
		// --------

		// Workshop callbacks
		STEAM_CALLBACK(rawrbox::SteamCALLBACKS, OnWorkshopItemInstalled, ItemInstalled_t, _CallbackWorkshopItemInstalled);
		STEAM_CALLBACK(rawrbox::SteamCALLBACKS, OnWorkshopItemRemoved, RemoteStoragePublishedFileUnsubscribed_t, _CallbackWorkshopItemRemoved);
		STEAM_CALLBACK(rawrbox::SteamCALLBACKS, OnWorkshopItemDownloaded, DownloadItemResult_t, _CallbackWorkshopItemDownloaded);
		// ---
#pragma GCC diagnostic pop
#pragma warning(pop)

		// WORKSHOP ----
		void OnWorkshopCreateItem(CreateItemResult_t* result, bool bIOFailure);
		void OnWorkshopUpdateItem(SubmitItemUpdateResult_t* result, bool bIOFailure);
		// -------------

		// QUERY ---
		void OnUGCQueryCompleted(SteamUGCQueryCompleted_t* pParam, bool bIOFailure);
		// --------

	public:
		rawrbox::Event<PublishedFileId_t> onModInstalled;
		rawrbox::Event<PublishedFileId_t> onModUpdated;
		rawrbox::Event<PublishedFileId_t> onModRemoved;

		static rawrbox::SteamCALLBACKS& getInstance() {
			static rawrbox::SteamCALLBACKS cl;
			return cl;
		}

		SteamCALLBACKS();

		// QUERY ---
		void addUGCQueryCallback(SteamAPICall_t apicall, const std::function<void(std::vector<SteamUGCDetails_t>)>& callback);
		// --------

		// WORKSHOP ---
		void addCreateItemCallback(SteamAPICall_t apicall, const std::function<void(CreateItemResult_t*)>& callback);
		void addUpdateItemCallback(SteamAPICall_t apicall, const std::function<void(SubmitItemUpdateResult_t*)>& callback);
		// ------------
	};
} // namespace rawrbox
