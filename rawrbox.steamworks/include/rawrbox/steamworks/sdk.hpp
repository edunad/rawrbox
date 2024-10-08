#pragma once
#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/logger.hpp>

#include <steam/steam_api.h>

#include <memory>

namespace rawrbox {
	struct SteamImage;

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
		static std::string getPersonaName(const CSteamID& id);
		static rawrbox::SteamImage getAvatar(const CSteamID& id, const rawrbox::AvatarSize& size);
		static std::vector<CSteamID> getFriends();
		//  -----------
	};
}; // namespace rawrbox
