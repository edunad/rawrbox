#pragma once

#include <rawrbox/steamworks/workshop/manager.hpp>

namespace rawrbox {
	class SteamStorageRequest {
	protected:
		UGCHandle_t _handle = k_UGCHandleInvalid;
		std::function<void(std::vector<uint8_t>)> _callback = nullptr;
		CCallResult<rawrbox::SteamStorageRequest, RemoteStorageDownloadUGCResult_t> _result = {};

	public:
		SteamStorageRequest(UGCHandle_t handle, SteamAPICall_t id, const std::function<void(std::vector<uint8_t>)>& callback);
		SteamStorageRequest(const SteamStorageRequest&) = delete;
		SteamStorageRequest& operator=(const SteamStorageRequest&) = delete;
		SteamStorageRequest(SteamStorageRequest&&) noexcept = delete;
		SteamStorageRequest& operator=(SteamStorageRequest&&) noexcept = delete;
		~SteamStorageRequest();

		void OnRequestCompleted(RemoteStorageDownloadUGCResult_t* pResult, bool bIOFailure);
	};
} // namespace rawrbox
