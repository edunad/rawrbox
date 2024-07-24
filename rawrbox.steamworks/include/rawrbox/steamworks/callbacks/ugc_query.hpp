#pragma once

#include <rawrbox/steamworks/workshop/manager.hpp>

namespace rawrbox {
	class SteamUGCQuery {
	protected:
		std::function<void(std::vector<rawrbox::WorkshopMod>)> _callback = nullptr;
		CCallResult<rawrbox::SteamUGCQuery, SteamUGCQueryCompleted_t> _result = {};

	public:
		SteamUGCQuery(SteamAPICall_t id, const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback);
		SteamUGCQuery(const SteamUGCQuery&) = delete;
		SteamUGCQuery& operator=(const SteamUGCQuery&) = delete;
		SteamUGCQuery(SteamUGCQuery&&) noexcept = delete;
		SteamUGCQuery& operator=(SteamUGCQuery&&) noexcept = delete;
		~SteamUGCQuery();

		void OnUGCQueryCompleted(SteamUGCQueryCompleted_t* pParam, bool bIOFailure);
	};
} // namespace rawrbox
