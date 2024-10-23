#include <rawrbox/steamworks/utils.hpp>
#include <rawrbox/utils/logger.hpp>

#include <steam/isteamutils.h>

namespace rawrbox {
	// PRIVATE -------------
	std::unique_ptr<rawrbox::Logger> SteamUTILS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SteamUTILS");
	// -------------

	rawrbox::SteamImage SteamUTILS::getImage(int handle) {
		if (SteamUtils() == nullptr) RAWRBOX_CRITICAL("SteamUtils not initialized");
		if (handle == 0) RAWRBOX_CRITICAL("Invalid handle");

		rawrbox::SteamImage image = {};

		bool success = SteamUtils()->GetImageSize(handle, &image.width, &image.height);
		if (!success) return image;

		const uint32_t uImageSizeInBytes = image.width * image.height * 4U;
		image.pixels.resize(uImageSizeInBytes);

		SteamUtils()->GetImageRGBA(handle, image.pixels.data(), uImageSizeInBytes);
		return image;
	}
} // namespace rawrbox
