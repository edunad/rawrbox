#pragma once

#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/logger.hpp>

#include <steam/steam_api.h>

#include <memory>

namespace rawrbox {
	struct SteamAvatar {
	public:
		std::vector<uint8_t> pixels;

		uint32_t width = 0;
		uint32_t height = 0;
	};

	enum class AvatarSize : uint32_t {
		SMALL = 0,
		MEDIUM,
		LARGE,
	};

	class SteamSDK {
	protected:
		static bool _initialized;
		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static rawrbox::Event<> onInitialized;

		static bool init();
		static void shutdown();
		static void update();

		// UTILS -----
		static bool isReady();

		static std::string getBuildVersion();
		static bool runningOnSteamDeck();
		// ------------

		// USER ---
		static CSteamID getSteamID();
		static rawrbox::SteamAvatar getAvatar(const CSteamID& id, const rawrbox::AvatarSize& size);
		static std::vector<CSteamID> getFriends();
		//  -----------
	};
}; // namespace rawrbox
