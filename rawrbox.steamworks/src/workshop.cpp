#include <rawrbox/steamworks/callbacks.hpp>
#include <rawrbox/steamworks/sdk.hpp>
#include <rawrbox/steamworks/workshop.hpp>
#include <rawrbox/utils/string.hpp>

#include <glaze/glaze.hpp>
#include <magic_enum.hpp>

#include <array>
#include <filesystem>

namespace rawrbox {
	// PRIVATE ----

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> SteamWORKSHOP::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SteamWORKSHOP");
	// ------

	bool SteamWORKSHOP::validateMod(PublishedFileId_t id) {
		return std::filesystem::exists(fmt::format("./mods/workshop_{}", std::to_string(id)));
	}

	void SteamWORKSHOP::loadMods(const std::vector<SteamUGCDetails_t>& details) {
		_logger->info("Loading workshop mods");

		std::vector<std::filesystem::path> loadedMods = {};
		for (const SteamUGCDetails_t& detail : details) {
			auto tags = rawrbox::StrUtils::split(detail.m_rgchTags, ",");
			if (tags.empty()) {
				_logger->warn("Missing tags for workshop item '{}', skipping mod..", detail.m_nPublishedFileId);
				continue;
			}

			auto status = getWorkshopModState(detail.m_nPublishedFileId);
			switch (status) {
				case WorkshopStatus::INSTALLED:
					_logger->info("Found workshop item '{}'", detail.m_nPublishedFileId);
					loadedMods.emplace_back(getWorkshopModFolder(detail.m_nPublishedFileId));
					break;

				case WorkshopStatus::NEEDS_UPDATE:
					{
						_logger->info("Updating workshop item '{}'", detail.m_nPublishedFileId);
						if (!downloadMod(detail.m_nPublishedFileId, true)) {
							throw _logger->error("Failed to update workshop item '{}'", detail.m_nPublishedFileId);
						}
					}
					break;
				case WorkshopStatus::DOWNLOADING:
				case WorkshopStatus::NONE:
				default:
					break;
			}
		}

		onModsLoaded(loadedMods);
	}

	// UPLOAD ------
	std::filesystem::path SteamWORKSHOP::moveToTempModFolder(const std::string& rootPath, const std::vector<std::filesystem::path>& uploadFiles) {
		// Create temp folder
		auto tempPath = std::filesystem::temp_directory_path().generic_string() + "tombmaker/mod_upload";

		std::filesystem::remove_all(tempPath);
		std::filesystem::create_directories(tempPath);

		for (const auto& p : uploadFiles) {
			std::string localpath = p.generic_string();
			localpath.erase(localpath.begin(), localpath.begin() + rootPath.length());

			if (std::filesystem::is_directory(p)) {
				std::filesystem::create_directories(fmt::format("{}/{}", tempPath, localpath));
			} else {
				std::filesystem::copy(p, fmt::format("{}/{}", tempPath, localpath));
			}
		}

		return tempPath;
	}

	void SteamWORKSHOP::updateItem(PublishedFileId_t id, const rawrbox::WorkshopModConfig& config, const std::filesystem::path& uploadPath, const std::function<void(SubmitItemUpdateResult_t*)>& callback, bool isNewMod) {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");

		UGCUpdateHandle_t updateHandle = SteamUGC()->StartItemUpdate(STEAMWORKS_APPID, id);
		if (updateHandle == k_UGCUpdateHandleInvalid) throw _logger->error("Failed to start item update");

		if (isNewMod) {
			auto description = config.description.value_or("Mod's description");

			if (!SteamUGC()->SetItemTitle(updateHandle, config.title.c_str())) throw _logger->error("Failed to set workshop item title");
			if (!SteamUGC()->SetItemDescription(updateHandle, description.c_str())) throw _logger->error("Failed to set workshop item description");
		}

		if (!config.id.has_value()) throw _logger->error("Missing workshop id");
		if (!SteamUGC()->AddItemKeyValueTag(updateHandle, "WORKSHOP_ID", config.id.value().c_str())) throw _logger->error("Failed to set workshop workshop type");

		std::string type = config.type.value_or("");
		if (type.empty()) {
			if (!SteamUGC()->AddItemKeyValueTag(updateHandle, "WORKSHOP_TYPE", type.c_str())) throw _logger->error("Failed to set workshop workshop type");
		}

		if (!SteamUGC()->SetItemContent(updateHandle, uploadPath.generic_string().c_str())) throw _logger->error("Failed to set workshop item content");

		if (config.preview.has_value()) {
			std::string previewPath = std::filesystem::absolute(fmt::format("{}/{}", uploadPath.generic_string(), config.preview.value())).generic_string();
			if (!SteamUGC()->SetItemPreview(updateHandle, previewPath.c_str())) throw _logger->error("Failed to set workshop item preview image");
		}

		// Allow custom ---
		onModUpdating(updateHandle, config);
		// ---------------------

		std::string changelog = fmt::format("Auto-update by engine v{}", rawrbox::SteamSDK::getBuildVersion());

		SteamAPICall_t apicall = SteamUGC()->SubmitItemUpdate(updateHandle, changelog.c_str());
		if (apicall == 0) throw _logger->error("Failed to update workshop item");

		SteamCALLBACKS::getInstance().addUpdateItemCallback(apicall, callback);
	}
	// -------------

	// -----------

	// PUBLIC -------
	rawrbox::Event<const std::vector<std::filesystem::path>&> SteamWORKSHOP::onModsLoaded = {};
	rawrbox::Event<const std::filesystem::path&> SteamWORKSHOP::onModInstalled = {};
	rawrbox::Event<const std::filesystem::path&> SteamWORKSHOP::onModRemoved = {};
	rawrbox::Event<const std::filesystem::path&> SteamWORKSHOP::onModUpdated = {};

	rawrbox::Event<const UGCUpdateHandle_t&, const rawrbox::WorkshopModConfig&> SteamWORKSHOP::onModUpdating = {};
	rawrbox::Event<const rawrbox::WorkshopModConfig&> SteamWORKSHOP::onModValidate = {};

	void SteamWORKSHOP::init() {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");

		// Register callback events ---
		SteamCALLBACKS::getInstance().onModInstalled += [](PublishedFileId_t id) {
			onModInstalled(getWorkshopModFolder(id));
		};

		SteamCALLBACKS::getInstance().onModUpdated += [](PublishedFileId_t id) {
			onModUpdated(getWorkshopModFolder(id));
		};

		SteamCALLBACKS::getInstance().onModRemoved += [](PublishedFileId_t id) {
			onModRemoved(getWorkshopModFolder(id));
		};
		// ----------------------

		std::vector<PublishedFileId_t> subs = getSubbedWorkshopMods();
		if (subs.empty()) {
			_logger->info("No workshop mods found, skipping load");
			onModsLoaded({});
			return;
		}

		UGCQueryHandle_t handle = SteamUGC()->CreateQueryUGCDetailsRequest(subs.data(), static_cast<uint32_t>(subs.size()));
		if (handle == k_UGCQueryHandleInvalid) throw _logger->error("Failed to request workshop items");
		if (!SteamUGC()->SetReturnChildren(handle, true)) return;

		SteamAPICall_t hSteamAPICall = SteamUGC()->SendQueryUGCRequest(handle);
		SteamCALLBACKS::getInstance().addUGCQueryCallback(hSteamAPICall, [](const std::vector<SteamUGCDetails_t>& details) {
			_logger->info("Found {} subscribed workshop items", details.size());
			loadMods(details);
		});
	}

	void SteamWORKSHOP::shutdown() {
		_logger.reset();
	}

	bool SteamWORKSHOP::downloadMod(PublishedFileId_t id, bool highPriority) {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");
		return SteamUGC()->DownloadItem(id, highPriority);
	}

	// UTILS -----
	std::vector<PublishedFileId_t> SteamWORKSHOP::getSubbedWorkshopMods() {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");
		uint32_t totalItems = SteamUGC()->GetNumSubscribedItems();

		std::vector<PublishedFileId_t> mods(totalItems);
		SteamUGC()->GetSubscribedItems(mods.data(), totalItems);

		return mods;
	}

	std::string SteamWORKSHOP::getWorkshopModFolder(PublishedFileId_t id) {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");

		uint64 size = 0;
		uint32 updateTimestamp = 0;
		std::array<char, 1024> buff = {};

		bool installed = SteamUGC()->GetItemInstallInfo(id, &size, buff.data(), sizeof(buff) / sizeof(char), &updateTimestamp);
		if (!installed) _logger->warn("Failed to access mod '{}', mod not installed?", id);

		return {buff.data()};
	}

	rawrbox::WorkshopStatus SteamWORKSHOP::getWorkshopModState(PublishedFileId_t id) {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");
		uint32 state = SteamUGC()->GetItemState(id);

		if ((state & EItemState::k_EItemStateSubscribed) == 0U) {
			return rawrbox::WorkshopStatus::NONE;
		}

		if ((state & EItemState::k_EItemStateInstalled) != 0U) {
			if ((state & (EItemState::k_EItemStateNeedsUpdate)) != 0U) {
				return rawrbox::WorkshopStatus::NEEDS_UPDATE;
			}

			if ((state & (EItemState::k_EItemStateDownloadPending)) != 0U) {
				return rawrbox::WorkshopStatus::NEEDS_UPDATE;
			}

			if ((state & EItemState::k_EItemStateDownloading) != 0U) {
				return rawrbox::WorkshopStatus::DOWNLOADING;
			}

			return rawrbox::WorkshopStatus::INSTALLED;
		}

		return rawrbox::WorkshopStatus::NONE;
	}
	// -------------

	// UPLOAD ------
	rawrbox::WorkshopModConfig SteamWORKSHOP::readConfig(const std::filesystem::path& rootPath) {
		rawrbox::WorkshopModConfig config = {};

		auto result = glz::read_file_json(config, fmt::format("{}/mod.json", rootPath.generic_string()), std::string{});
		if (result.ec != glz::error_code::none) {
			throw _logger->error("Failed to read 'mod.json' file {{}}", magic_enum::enum_name(result.ec));
		}

		// Update config id --
		if (!config.id.has_value()) {
			config.id = rootPath.filename().generic_string();
		}
		// ------------------

		// Validate preview
		if (config.preview.has_value()) {
			auto previewPath = std::filesystem::absolute(rootPath / config.preview.value());

			if (!std::filesystem::exists(previewPath)) {
				throw _logger->error("Failed to find 'preview' image '{}'", previewPath.generic_string());
			}
		}
		// ----------------

		onModValidate(config);

		return config;
	}

	std::vector<std::filesystem::path> SteamWORKSHOP::validateFiles(const std::filesystem::path& rootPath, const std::vector<std::string>& ignore) {
		std::vector<std::filesystem::path> uploadFiles = {};

		auto validateAndCollectFiles = [&ignore, &uploadFiles](auto iterator) {
			for (const std::filesystem::directory_entry& entry : iterator) {
				const auto& filePath = entry.path();

				auto fileExtension = filePath.extension().generic_string();
				auto fileName = filePath.filename().generic_string();

				// Check if the current item should be ignored
				if (!ignore.empty() && std::find(ignore.begin(), ignore.end(), fileName) != ignore.end()) continue;

				// Handle directories differently
				if (entry.is_directory()) {
					uploadFiles.push_back(filePath);
					continue; // Skip further processing for directories
				}

				// Validate file extensions against whitelist
				if (std::find(fileWhitelist.begin(), fileWhitelist.end(), fileExtension) == fileWhitelist.end()) {
					_logger->warn("Invalid file '{}'", fileName);
					continue;
				}

				uploadFiles.push_back(filePath);
			}
		};

		validateAndCollectFiles(std::filesystem::recursive_directory_iterator(rootPath));
		if (uploadFiles.empty()) throw _logger->error("\nCannot upload empty mod!");

		return uploadFiles;
	}

	std::string SteamWORKSHOP::checkErrors(EResult result) {
		switch (result) {
			case EResult::k_EResultOK:
				return "";
			case EResult::k_EResultAccessDenied:
			case EResult::k_EResultInsufficientPrivilege:
				return "User doesn't have permission to create workshop items";
			case EResult::k_EResultInvalidParam:
				return "Invalid app ID";
			case EResult::k_EResultFileNotFound:
				return "Invalid workshop id, item not found";
			case EResult::k_EResultLimitExceeded:
				return "Image too big (needs to be < 1 mb)";
			default:
				return "Something went wrong";
		}
	}

	void SteamWORKSHOP::updateWorkshop(PublishedFileId_t id, const rawrbox::WorkshopModConfig& config, const std::filesystem::path& rootPath, const std::vector<std::filesystem::path>& files, const std::function<void(SubmitItemUpdateResult_t*)>& onComplete) {
		if (files.empty()) throw _logger->error("Cannot upload empty mod!");

		auto tempFolder = moveToTempModFolder(rootPath.generic_string(), files);
		_logger->info("Created temp folder '{}'", tempFolder.generic_string());

		updateItem(
		    id,
		    config, tempFolder, [onComplete](SubmitItemUpdateResult_t* result) {
			    onComplete(result);
		    },
		    false);
	}

	void SteamWORKSHOP::createWorkshop(rawrbox::WorkshopModConfig& config, const std::filesystem::path& rootPath, const std::vector<std::filesystem::path>& files, const std::function<void(SubmitItemUpdateResult_t*)>& onComplete) {
		if (files.empty()) throw _logger->error("Cannot upload empty mod!");

		SteamAPICall_t apicall = SteamUGC()->CreateItem(STEAMWORKS_APPID, k_EWorkshopFileTypeCommunity);
		if (apicall == 0) throw _logger->error("Failed to create workshop item");

		SteamCALLBACKS::getInstance().addCreateItemCallback(apicall, [&config, &rootPath, &files, onComplete](CreateItemResult_t* result) {
			if (result->m_bUserNeedsToAcceptWorkshopLegalAgreement) {
				throw _logger->error("User needs to accept steam's workshop agreement (https://steamcommunity.com/sharedfiles/workshoplegalagreement)");
			}

			auto err = checkErrors(result->m_eResult);
			if (!err.empty()) throw _logger->error("{}", err);

			auto tempFolder = moveToTempModFolder(rootPath.generic_string(), files);
			_logger->info("Created temp folder '{}'", tempFolder.generic_string());

			updateItem(
			    result->m_nPublishedFileId, config, tempFolder, [onComplete](SubmitItemUpdateResult_t* result) {
				    onComplete(result);
			    },
			    true);
		});
	}

	// -------------
} // namespace rawrbox
