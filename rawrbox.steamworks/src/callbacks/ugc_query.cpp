
#include <rawrbox/steamworks/callbacks/ugc_query.hpp>

#include <array>

namespace rawrbox {
	SteamUGCQuery::SteamUGCQuery(SteamAPICall_t id, const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback) : _callback(callback) {
		this->_result.Set(id, this, &rawrbox::SteamUGCQuery::OnUGCQueryCompleted);
	}
	SteamUGCQuery::~SteamUGCQuery() { this->_result.Cancel(); }

	void SteamUGCQuery::OnUGCQueryCompleted(SteamUGCQueryCompleted_t* pParam, bool bIOFailure) {
		std::vector<rawrbox::WorkshopMod> details = {};

		if (bIOFailure || pParam->m_eResult != k_EResultOK || pParam->m_unNumResultsReturned <= 0) {
			this->_callback(details);
			return;
		}

		for (uint32_t i = 0; i < pParam->m_unNumResultsReturned; i++) {
			rawrbox::WorkshopMod detail = {};

			uint32_t count = SteamUGC()->GetQueryUGCNumKeyValueTags(pParam->m_handle, i);
			for (uint32_t tag = 0; tag < count; tag++) {
				std::array<char, 256> key = {};
				std::array<char, 256> val = {};

				if (!SteamUGC()->GetQueryUGCKeyValueTag(pParam->m_handle, i, tag, key.data(), static_cast<uint32_t>(key.size()), val.data(), static_cast<uint32_t>(val.size()))) continue;
				detail.keyVals[key.data()] = val.data();
			}

			if (SteamUGC()->GetQueryUGCResult(pParam->m_handle, i, &detail)) {
				if (detail.m_eResult == k_EResultOK && detail.m_eFileType == k_EWorkshopFileTypeCommunity) {
					detail.modId = detail.keyVals.contains("MOD_ID") ? detail.keyVals["MOD_ID"] : std::to_string(detail.m_nPublishedFileId);
					detail.installPath = rawrbox::SteamWORKSHOP::getWorkshopModFolder(detail.m_nPublishedFileId);
					details.push_back(detail);
				}
			}
		}

		SteamUGC()->ReleaseQueryUGCRequest(pParam->m_handle);
		this->_callback(details);
	}
} // namespace rawrbox
