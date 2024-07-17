#pragma once

#include <rawrbox/steamworks/callbacks.hpp>
#include <rawrbox/steamworks/workshop/storage.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/tasker.hpp>

#include <steam/isteamremotestorage.h>

#include <functional>
#include <memory>

namespace rawrbox {
	struct DownloadRequest : public rawrbox::Task {
		UGCHandle_t handle = 0;
		std::function<void(std::vector<uint8_t>)> callback = nullptr;
		uint32_t priority = 0;

		DownloadRequest(UGCHandle_t handle, const std::function<void(std::vector<uint8_t>)>& callback, uint32_t priority = 0)
		    : handle(handle), callback(callback), priority(priority) {}

		void run(const std::function<void()>& done) override {
			SteamAPICall_t hAPICall = SteamRemoteStorage()->UGCDownload(handle, priority);
			if (hAPICall == k_uAPICallInvalid) return;

			rawrbox::SteamCALLBACKS::getInstance().addUGCRequest(handle, hAPICall, [done, this](const std::vector<uint8_t>& data) {
				callback(data);
				done();
			});
		}

		void cancel() override {
			rawrbox::SteamCALLBACKS::getInstance().cancelUGCRequest(handle);
		}
	};

	class SteamSTORAGE {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;
		static rawrbox::Tasker<rawrbox::DownloadRequest> _downloadQueue;

		static std::unordered_map<std::string, std::vector<uint8_t>> _cache;

		static bool isCached(UGCHandle_t handle);
		static std::vector<uint8_t> getCache(UGCHandle_t handle);

	public:
		static void init();
		static void shutdown();

		// DOWNLOAD ---
		static void download(UGCHandle_t handle, const std::function<void(std::vector<uint8_t>)>& callback, uint32_t priority = 0U);
		//-------------
	};
} // namespace rawrbox
