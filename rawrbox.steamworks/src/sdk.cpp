
#include <rawrbox/steamworks/callbacks/manager.hpp>
#include <rawrbox/steamworks/input.hpp>
#include <rawrbox/steamworks/sdk.hpp>
#include <rawrbox/steamworks/utils.hpp>
#include <rawrbox/steamworks/workshop/manager.hpp>
#include <rawrbox/steamworks/workshop/storage.hpp>

#ifdef _WIN32
	#include <Windows.h>
	#include <debugapi.h>
#endif

#include <fmt/printf.h>

#include <array>

namespace rawrbox {
	extern "C" void __cdecl SteamAPIDebugTextHook(int nSeverity, const char* pchDebugText) {
#ifdef _WIN32
		::OutputDebugString(pchDebugText);
#endif
		if (nSeverity >= 1) fmt::print("[SteamSDK][{}] {}\n", nSeverity, pchDebugText);
	}

	// PRIVATE ----------
	bool SteamSDK::_initialized = false;

	std::unique_ptr<rawrbox::Logger> SteamSDK::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SteamSDK");
	// ------------------

	// PUBLIC ------
	rawrbox::Event<> SteamSDK::onInitialized = {};

	bool SteamSDK::init() {
		if (_initialized) CRITICAL_RAWRBOX("SteamSDK already initialized");

#ifdef NDEBUG
		if (SteamAPI_RestartAppIfNecessary(STEAMWORKS_APPID)) return false;
#endif

		if (!SteamAPI_Init()) return false;
		if (!SteamUser()->BLoggedOn()) return false;

		// Steam debug msgs
		SteamUtils()->SetWarningMessageHook(&SteamAPIDebugTextHook);

		// Callbacks
		rawrbox::SteamCALLBACKS::getInstance().init();

		// Input
		rawrbox::SteamINPUT::init();

		// Storage
		rawrbox::SteamSTORAGE::init();

		_initialized = true;
		onInitialized();

		return true;
	}

	void SteamSDK::shutdown() {
		rawrbox::SteamCALLBACKS::getInstance().shutdown();
		rawrbox::SteamWORKSHOP::shutdown();
		rawrbox::SteamINPUT::shutdown();

		SteamAPI_Shutdown();
	}

	void SteamSDK::update() {
		if (!isReady()) return;

		SteamAPI_RunCallbacks();
		rawrbox::SteamINPUT::update();
	}
	// ------------

	// UTILS -----
	bool SteamSDK::isReady() {
		return _initialized && SteamUser() != nullptr && SteamMatchmaking() != nullptr && SteamUGC() != nullptr && SteamUtils() != nullptr;
	}

	std::string SteamSDK::getBuildVersion() {
		if (SteamApps() == nullptr) CRITICAL_RAWRBOX("SteamApps not initialized");

		int id = SteamApps()->GetAppBuildId();
		std::string str = fmt::format("BUILD {}", id);

		std::array<char, 1024> buff = {};
		if (SteamApps()->GetCurrentBetaName(buff.data(), sizeof(buff) / sizeof(char))) {
			str += fmt::format("- BRANCH {}", buff.data());
		}

		return str;
	}

	bool SteamSDK::runningOnSteamDeck() { return SteamUtils()->IsSteamRunningOnSteamDeck(); }
	// ------------

	// USER ---
	CSteamID SteamSDK::getSteamID() {
		if (SteamUser() == nullptr) CRITICAL_RAWRBOX("SteamUser not initialized");
		return SteamUser()->GetSteamID();
	}

	std::string SteamSDK::getPersonaName(const CSteamID& id) {
		if (SteamFriends() == nullptr) CRITICAL_RAWRBOX("SteamFriends not initialized");

		if (!SteamFriends()->RequestUserInformation(id, true)) {
			const auto* name = SteamFriends()->GetFriendPersonaName(id);
			return name;
		}

		return "";
	}

	rawrbox::SteamImage SteamSDK::getAvatar(const CSteamID& id, const rawrbox::AvatarSize& size) {
		if (SteamUser() == nullptr) CRITICAL_RAWRBOX("SteamUser not initialized");
		rawrbox::SteamImage avatar = {};

		int ptrId = 0;
		switch (size) {
			case rawrbox::AvatarSize::SMALL: ptrId = SteamFriends()->GetSmallFriendAvatar(id); break;
			case rawrbox::AvatarSize::MEDIUM: ptrId = SteamFriends()->GetMediumFriendAvatar(id); break;
			case rawrbox::AvatarSize::LARGE: ptrId = SteamFriends()->GetLargeFriendAvatar(id); break;
		}

		if (ptrId == 0) return avatar;
		return rawrbox::SteamUTILS::getImage(ptrId);
	}

	std::vector<CSteamID> SteamSDK::getFriends() {
		if (SteamFriends() == nullptr) CRITICAL_RAWRBOX("SteamUser not initialized");
		int friendCount = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);

		std::vector<CSteamID> friends = {};
		friends.resize(friendCount);

		for (int i = 0; i < friendCount; i++) {
			friends[i] = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
		}

		return friends;
	}
	// -----------
}; // namespace rawrbox
