#include <rawrbox/steamworks/callbacks/manager.hpp>
#include <rawrbox/steamworks/sdk.hpp>
#include <rawrbox/steamworks/workshop/manager.hpp>
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

	void SteamWORKSHOP::loadMods(const std::vector<rawrbox::WorkshopMod>& details) {
		_logger->info("Loading workshop mods");

		std::vector<rawrbox::WorkshopMod> loadedMods = {};
		for (const rawrbox::WorkshopMod& detail : details) {
			auto tags = rawrbox::StrUtils::split(detail.m_rgchTags, ",");
			if (tags.empty()) {
				_logger->warn("Missing tags for workshop item '{}', skipping mod..", detail.m_nPublishedFileId);
				continue;
			}

			auto status = getWorkshopModState(detail.m_nPublishedFileId);
			switch (status) {
				case WorkshopStatus::INSTALLED:
					_logger->info("Found workshop item '{}'", detail.m_nPublishedFileId);
					loadedMods.emplace_back(detail);
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
				case WorkshopStatus::UNKNOWN:
				default:
					break;
			}
		}

		onModsLoaded(loadedMods);
	}

	// UTILS ---
	void SteamWORKSHOP::patchConfig(const std::filesystem::path& rootPath, const rawrbox::WorkshopModConfig& config) {
		auto configPath = fmt::format("{}/mod.json", rootPath.generic_string());
		if (!std::filesystem::exists(configPath)) throw _logger->error("Missing 'mod.json' file");

		auto ec = glz::write_file_json<glz::opts{.comments = 1U, .prettify = 1U, .allow_conversions = 1U}>(config, configPath, std::string{});
		if (ec != glz::error_code::none) throw _logger->error("Failed to patch settings '{}'", configPath);

		_logger->success("Patched mod config");
	}
	// -------

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
		if (!config.id.has_value()) throw _logger->error("Invalid mod id");

		UGCUpdateHandle_t updateHandle = SteamUGC()->StartItemUpdate(STEAMWORKS_APPID, id);
		if (updateHandle == k_UGCUpdateHandleInvalid) throw _logger->error("Failed to start item update");

		// SETUP TITLE AND DESCRIPTION ----
		if (isNewMod) {
			auto description = config.description.value_or("Mod's description");

			if (!SteamUGC()->SetItemTitle(updateHandle, config.title.c_str())) throw _logger->error("Failed to set workshop item title");
			if (!SteamUGC()->SetItemDescription(updateHandle, description.c_str())) throw _logger->error("Failed to set workshop item description");
		}
		// ------

		// SETUP KEY VALUES ----
		std::string type = config.type.value_or("MOD");
		if (!type.empty()) {
			if (!SteamUGC()->AddItemKeyValueTag(updateHandle, "MOD_TYPE", type.c_str())) throw _logger->error("Failed to set workshop type");
		}

		if (!SteamUGC()->AddItemKeyValueTag(updateHandle, "MOD_ID", config.id.value().c_str())) throw _logger->error("Failed to set workshop mod id");
		// ----------------------

		// SETUP DEPENDENCIES ----
		if (config.dependencies.has_value()) {
			for (const auto& dep : config.dependencies.value()) {
				PublishedFileId_t depId = std::strtoull(dep.c_str(), nullptr, 10);
				if (depId == id) throw _logger->error("Cannot add self as a dependency");

				if (SteamUGC()->AddDependency(id, depId) == 0) throw _logger->error("Failed to set workshop dependency {}", dep);
			}
		}
		// ----------------

		// SETUP CONTENT ----
		if (!SteamUGC()->SetItemContent(updateHandle, uploadPath.generic_string().c_str())) throw _logger->error("Failed to set workshop item content");
		// ------

		// SETUP PREVIEW ----
		if (config.preview.has_value()) {
			std::string previewPath = std::filesystem::absolute(fmt::format("{}/{}", uploadPath.generic_string(), config.preview.value())).generic_string();
			if (!SteamUGC()->SetItemPreview(updateHandle, previewPath.c_str())) throw _logger->error("Failed to set workshop item preview image");
		}
		// -------------

		// CUSTOM CONFIG ---
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
	rawrbox::Event<const std::vector<rawrbox::WorkshopMod>&> SteamWORKSHOP::onModsLoaded = {};
	rawrbox::Event<PublishedFileId_t> SteamWORKSHOP::onModInstalled = {};
	rawrbox::Event<PublishedFileId_t> SteamWORKSHOP::onModUnSubscribed = {};
	rawrbox::Event<PublishedFileId_t> SteamWORKSHOP::onModSubscribed = {};
	rawrbox::Event<PublishedFileId_t> SteamWORKSHOP::onModUpdated = {};

	rawrbox::Event<const UGCUpdateHandle_t&, const rawrbox::WorkshopModConfig&> SteamWORKSHOP::onModUpdating = {};
	rawrbox::Event<const rawrbox::WorkshopModConfig&> SteamWORKSHOP::onModValidate = {};

	void SteamWORKSHOP::init() {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");

		// Register callback events ---
		SteamCALLBACKS::getInstance().onModInstalled += [](PublishedFileId_t id) {
			onModInstalled(id);
		};

		SteamCALLBACKS::getInstance().onModUpdated += [](PublishedFileId_t id) {
			onModUpdated(id);
		};

		SteamCALLBACKS::getInstance().onModUnSubscribed += [](PublishedFileId_t id) {
			onModUnSubscribed(id);
		};

		SteamCALLBACKS::getInstance().onModSubscribed += [](PublishedFileId_t id) {
			onModSubscribed(id);
		};
		// ----------------------

		std::vector<PublishedFileId_t> subs = getSubbedWorkshopMods();
		if (subs.empty()) {
			_logger->info("No workshop mods found, skipping load");
			onModsLoaded({});
			return;
		}

		_logger->info("Loading {} workshop mods", subs.size());
		queryMods(subs, [subs](const std::vector<rawrbox::WorkshopMod>& details) {
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

	std::filesystem::path SteamWORKSHOP::getWorkshopModFolder(PublishedFileId_t id) {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");

		uint64 size = 0;
		uint32 updateTimestamp = 0;
		std::array<char, 1024> buff = {};
		SteamUGC()->GetItemInstallInfo(id, &size, buff.data(), static_cast<uint32_t>(buff.size()), &updateTimestamp);

		return {buff.data()};
	}

	rawrbox::WorkshopStatus SteamWORKSHOP::getWorkshopModState(PublishedFileId_t id) {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");
		uint32 state = SteamUGC()->GetItemState(id);

		if ((state & EItemState::k_EItemStateSubscribed) == 0U) {
			return rawrbox::WorkshopStatus::UNKNOWN; // Not subscribed
		}

		if ((state & (EItemState::k_EItemStateDownloadPending)) != 0U) {
			return rawrbox::WorkshopStatus::NEEDS_UPDATE;
		}

		if ((state & EItemState::k_EItemStateDownloading) != 0U) {
			return rawrbox::WorkshopStatus::DOWNLOADING;
		}

		if ((state & EItemState::k_EItemStateInstalled) != 0U) {
			if ((state & (EItemState::k_EItemStateNeedsUpdate)) != 0U) {
				return rawrbox::WorkshopStatus::NEEDS_UPDATE;
			}

			return rawrbox::WorkshopStatus::INSTALLED;
		}

		return rawrbox::WorkshopStatus::UNKNOWN;
	}

	// -------------

	// QUERIES ----
	void SteamWORKSHOP::queryUserMods(const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback, EUserUGCList type, const std::vector<std::string>& tags, uint32_t page) {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");

		auto id = SteamUser()->GetSteamID().GetAccountID();
		UGCQueryHandle_t handle = SteamUGC()->CreateQueryUserUGCRequest(id, type, k_EUGCMatchingUGCType_Items, k_EUserUGCListSortOrder_CreationOrderDesc, STEAMWORKS_APPID, STEAMWORKS_APPID, page);
		if (handle == k_UGCQueryHandleInvalid) throw _logger->error("Failed to request workshop items");

		// TAGS -----
		for (const auto& tag : tags) {
			if (!SteamUGC()->AddRequiredTag(handle, tag.c_str())) {
				_logger->warn("Failed to add tag '{}' to query search", tag);
				continue;
			}
		}
		// -----------

		if (!SteamUGC()->SetReturnKeyValueTags(handle, true)) throw _logger->error("Failed to request key values");

		SteamAPICall_t hSteamAPICall = SteamUGC()->SendQueryUGCRequest(handle);
		SteamCALLBACKS::getInstance().addUGCQueryCallback(hSteamAPICall, [callback](const std::vector<rawrbox::WorkshopMod>& details) {
			callback(details);
		});
	}

	void SteamWORKSHOP::queryMods(const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback, EUGCQuery type, const std::vector<std::string>& tags, uint32_t page) {
		if (SteamUGC() == nullptr) throw _logger->error("SteamUGC not initialized");

		UGCQueryHandle_t handle = SteamUGC()->CreateQueryAllUGCRequest(type, k_EUGCMatchingUGCType_Items, STEAMWORKS_APPID, STEAMWORKS_APPID, page);
		if (handle == k_UGCQueryHandleInvalid) throw _logger->error("Failed to request workshop items");

		// TAGS -----
		for (const auto& tag : tags) {
			if (!SteamUGC()->AddRequiredTag(handle, tag.c_str())) {
				_logger->warn("Failed to add tag '{}' to query search", tag);
				continue;
			}
		}
		// -----------

		if (!SteamUGC()->SetReturnKeyValueTags(handle, true)) throw _logger->error("Failed to request key values");

		SteamAPICall_t hSteamAPICall = SteamUGC()->SendQueryUGCRequest(handle);
		SteamCALLBACKS::getInstance().addUGCQueryCallback(hSteamAPICall, [callback](const std::vector<rawrbox::WorkshopMod>& details) {
			callback(details);
		});
	}

	void SteamWORKSHOP::queryMods(std::vector<PublishedFileId_t> ids, const std::function<void(std::vector<rawrbox::WorkshopMod>)>& callback) {
		UGCQueryHandle_t handle = SteamUGC()->CreateQueryUGCDetailsRequest(ids.data(), static_cast<uint32_t>(ids.size()));
		if (handle == k_UGCQueryHandleInvalid) throw _logger->error("Failed to request workshop items");
		if (!SteamUGC()->SetReturnKeyValueTags(handle, true)) throw _logger->error("Failed to request workshop items");

		SteamAPICall_t hSteamAPICall = SteamUGC()->SendQueryUGCRequest(handle);
		SteamCALLBACKS::getInstance().addUGCQueryCallback(hSteamAPICall, [callback](const std::vector<rawrbox::WorkshopMod>& details) {
			callback(details);
		});
	}
	// -------------

	// UPLOAD ------
	rawrbox::WorkshopModConfig SteamWORKSHOP::readConfig(const std::filesystem::path& rootPath) {
		rawrbox::WorkshopModConfig config = {};

		auto result = glz::read_file_json<glz::opts{.comments = 1U}>(config, fmt::format("{}/mod.json", rootPath.generic_string()), std::string{});
		if (result.ec != glz::error_code::none) {
			throw _logger->error("Failed to read 'mod.json' file {{}}", magic_enum::enum_name(result.ec));
		}

		// Always override and fill id
		if (!config.id.has_value()) {
			config.id = rootPath.filename().generic_string();
			_logger->info("Missing mod id, using {}", config.id.value());
		}
		// ---------------

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
		    config, rootPath, [onComplete](SubmitItemUpdateResult_t* result) {
			    onComplete(result);
		    },
		    false);
	}

	void SteamWORKSHOP::createWorkshop(rawrbox::WorkshopModConfig& config, const std::filesystem::path& rootPath, const std::vector<std::filesystem::path>& files, const std::function<void(SubmitItemUpdateResult_t*)>& onComplete) {
		if (files.empty()) throw _logger->error("Cannot upload empty mod!");
		if (config.__workshop_id__.has_value()) throw _logger->error("Mod already has a workshop id");

		SteamAPICall_t apicall = SteamUGC()->CreateItem(STEAMWORKS_APPID, k_EWorkshopFileTypeCommunity);
		if (apicall == 0) throw _logger->error("Failed to create workshop item");

		SteamCALLBACKS::getInstance().addCreateItemCallback(apicall, [&config, &rootPath, &files, onComplete](CreateItemResult_t* result) {
			if (result->m_bUserNeedsToAcceptWorkshopLegalAgreement) {
				throw _logger->error("User needs to accept steam's workshop agreement (https://steamcommunity.com/sharedfiles/workshoplegalagreement)");
			}

			auto err = checkErrors(result->m_eResult);
			if (!err.empty()) throw _logger->error("{}", err);

			// PATCH CONFIG ----
			config.__workshop_id__ = std::to_string(result->m_nPublishedFileId);
			patchConfig(rootPath, config);
			// ------

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
