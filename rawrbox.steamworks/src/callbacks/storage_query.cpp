
#include <rawrbox/steamworks/callbacks/storage_query.hpp>
#include <rawrbox/utils/threading.hpp>

namespace rawrbox {
	SteamStorageRequest::SteamStorageRequest(UGCHandle_t handle, SteamAPICall_t id, const std::function<void(std::vector<uint8_t>)>& callback) : _handle(handle), _callback(callback) {
		this->_result.Set(id, this, &rawrbox::SteamStorageRequest::OnRequestCompleted);
	}

	SteamStorageRequest::~SteamStorageRequest() {
		this->_handle = k_UGCHandleInvalid;
		this->_callback = nullptr;
		this->_result.Cancel();
	}

	void SteamStorageRequest::OnRequestCompleted(RemoteStorageDownloadUGCResult_t* pResult, bool bIOFailure) {
		if (SteamRemoteStorage() == nullptr) throw std::runtime_error("SteamRemoteStorage is null");

		if (this->_callback == nullptr ||
		    bIOFailure || pResult->m_eResult != k_EResultOK) {
			return;
		}

		rawrbox::ASYNC::run([pResult, this]() {
			std::vector<uint8_t> buffer(pResult->m_nSizeInBytes);
			SteamRemoteStorage()->UGCRead(this->_handle, buffer.data(), pResult->m_nSizeInBytes, 0, k_EUGCRead_ContinueReadingUntilFinished);
			// TODO: SUPPORT LONGER UGC CONTENT
			this->_callback(buffer);
		});
	}

} // namespace rawrbox
