#pragma once

#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/logger.hpp>

#include <steam/steam_api.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace rawrbox {

	enum class WorkshopStatus : uint32_t {
		NONE = 0U,
		INSTALLED,
		NEEDS_UPDATE,
		DOWNLOADING,
		INVALID
	};

	static const std::vector<std::string> fileWhitelist = {
	    // Lua only
	    ".luau",

	    // Supported sounds
	    ".ogg",
	    ".wav",
	    ".mp3",
	    ".mp4",

	    // Supported images
	    ".png",
	    ".jpg",
	    ".jpeg",
	    ".bmp",
	    ".gif",

	    // Supported models
	    ".fbx",
	    ".obj",
	    ".mtl",
	    ".dae",
	    ".3ds",
	    ".mdl",
	    ".md5mesh",

	    // Supported file formats
	    ".json",
	    ".txt",
	    ".md",
	    ".map"};

	struct WorkshopModConfig {
		std::optional<std::string> type;

		std::string title;
		std::string version;
		std::string author;

		std::optional<std::string> description;
		std::optional<std::string> preview;
		std::optional<std::string> repo;

		std::optional<std::vector<std::string>> tags;
		std::optional<std::vector<std::string>> ignore;
	};

	struct WorkshopMod : public SteamUGCDetails_t {
		std::unordered_map<std::string, std::string> keyVals;
		std::filesystem::path installPath;
	};

	class SteamWORKSHOP {
	protected:
		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// ------------

		[[nodiscard]] static bool validateMod(PublishedFileId_t id);
		static void loadMods(const std::vector<rawrbox::WorkshopMod>& details);

		// UPLOAD ------
		[[nodiscard]] static std::filesystem::path moveToTempModFolder(const std::string& rootPath, const std::vector<std::filesystem::path>& uploadFiles);
		static void updateItem(PublishedFileId_t id, const rawrbox::WorkshopModConfig& config, const std::filesystem::path& uploadPath, const std::function<void(SubmitItemUpdateResult_t*)>& callback, bool isNewMod = false);
		// -------------
	public:
		static rawrbox::Event<const std::vector<rawrbox::WorkshopMod>&> onModsLoaded;

		static rawrbox::Event<PublishedFileId_t> onModInstalled;
		static rawrbox::Event<PublishedFileId_t> onModUnSubscribed;
		static rawrbox::Event<PublishedFileId_t> onModSubscribed;
		static rawrbox::Event<PublishedFileId_t> onModUpdated;

		static rawrbox::Event<const UGCUpdateHandle_t&, const rawrbox::WorkshopModConfig&> onModUpdating;
		static rawrbox::Event<const rawrbox::WorkshopModConfig&> onModValidate;

		static void init();
		static void shutdown();

		[[nodiscard]] static bool downloadMod(PublishedFileId_t id, bool highPriority = false);

		// UTILS -----
		[[nodiscard]] static std::vector<PublishedFileId_t> getSubbedWorkshopMods();

		[[nodiscard]] static std::string getWorkshopModFolder(PublishedFileId_t id);
		[[nodiscard]] static rawrbox::WorkshopStatus getWorkshopModState(PublishedFileId_t id);

		[[nodiscard]] static const std::string& getKeyVal(PublishedFileId_t id, const std::string& key);
		// -------------

		// QUERIES ---
		static void queryUserMods(const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback, EUserUGCList type = k_EUserUGCList_Subscribed, const std::vector<std::string>& tags = {}, uint32_t page = 1U);
		static void queryMods(const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback, EUGCQuery type = k_EUGCQuery_RankedByVote, const std::vector<std::string>& tags = {}, uint32_t page = 1U);
		//-------------

		// UPLOAD ------
		[[nodiscard]] static rawrbox::WorkshopModConfig readConfig(const std::filesystem::path& rootPath);
		[[nodiscard]] static std::vector<std::filesystem::path> validateFiles(const std::filesystem::path& rootPath, const std::vector<std::string>& ignore);

		[[nodiscard]] static std::string checkErrors(EResult result);

		static void updateWorkshop(PublishedFileId_t id, const rawrbox::WorkshopModConfig& config, const std::filesystem::path& rootPath, const std::vector<std::filesystem::path>& files, const std::function<void(SubmitItemUpdateResult_t*)>& onComplete);
		static void createWorkshop(rawrbox::WorkshopModConfig& config, const std::filesystem::path& rootPath, const std::vector<std::filesystem::path>& files, const std::function<void(SubmitItemUpdateResult_t*)>& onComplete);
		// -------------
	};
} // namespace rawrbox
