#pragma once

#include <rawrbox/steamworks/callbacks/storage_query.hpp>
#include <rawrbox/steamworks/callbacks/ugc_query.hpp>

#include <steam/steam_api.h>

#include <functional>
#include <memory>
#include <vector>

namespace rawrbox {
	class SteamCALLBACKS {
	protected:
		// QUERY ----
		std::unordered_map<SteamAPICall_t, std::unique_ptr<rawrbox::SteamUGCQuery>> _ugcQueries = {};
		std::unordered_map<SteamAPICall_t, std::unique_ptr<rawrbox::SteamStorageRequest>> _ugcStorageQueries = {};
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
		STEAM_CALLBACK(rawrbox::SteamCALLBACKS, OnWorkshopItemUnSubscribed, RemoteStoragePublishedFileUnsubscribed_t, _CallbackWorkshopItemUnSubscribed);
		STEAM_CALLBACK(rawrbox::SteamCALLBACKS, OnWorkshopItemSubscribed, RemoteStoragePublishedFileSubscribed_t, _CallbackWorkshopItemSubscribed);
		STEAM_CALLBACK(rawrbox::SteamCALLBACKS, OnWorkshopItemDownloaded, DownloadItemResult_t, _CallbackWorkshopItemDownloaded);
		// ---
#pragma GCC diagnostic pop
#pragma warning(pop)

		// WORKSHOP ----
		void OnWorkshopCreateItem(CreateItemResult_t* result, bool bIOFailure);
		void OnWorkshopUpdateItem(SubmitItemUpdateResult_t* result, bool bIOFailure);
		// -------------

		bool _initialized = false;

	public:
		rawrbox::Event<PublishedFileId_t> onModSubscribed;
		rawrbox::Event<PublishedFileId_t> onModUnSubscribed;
		rawrbox::Event<PublishedFileId_t> onModInstalled;
		rawrbox::Event<PublishedFileId_t> onModUpdated;

		static rawrbox::SteamCALLBACKS& getInstance() {
			static rawrbox::SteamCALLBACKS cl;
			return cl;
		}

		SteamCALLBACKS();

		void init();
		void shutdown();

		// THREADING ---
		void runOnThread(const std::function<void()>& thread);
		// -------------

		// QUERY ---
		void addUGCQueryCallback(SteamAPICall_t apicall, const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback);
		// --------

		// WORKSHOP ---
		void addCreateItemCallback(SteamAPICall_t apicall, const std::function<void(CreateItemResult_t*)>& callback);
		void addUpdateItemCallback(SteamAPICall_t apicall, const std::function<void(SubmitItemUpdateResult_t*)>& callback);
		// ------------

		// STORAGE ---
		void cancelUGCRequest(SteamAPICall_t handle);
		void cancelAllUGCRequest();

		void addUGCRequest(UGCHandle_t handle, SteamAPICall_t apicall, const std::function<void(std::vector<uint8_t>)>& callback);
		// -----------
	};
} // namespace rawrbox
