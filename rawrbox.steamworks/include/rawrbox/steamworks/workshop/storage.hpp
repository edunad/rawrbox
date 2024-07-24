#pragma once
#include <rawrbox/utils/logger.hpp>

#include <steam/isteamremotestorage.h>

#include <functional>
#include <memory>

namespace rawrbox {
	class SteamSTORAGE {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static void init();
		static void shutdown();

		static void cancelRequests();

		// DOWNLOAD ---
		static void download(UGCHandle_t handle, const std::function<void(std::vector<uint8_t>)>& callback, uint32_t priority = 0U);
		//-------------
	};
} // namespace rawrbox
