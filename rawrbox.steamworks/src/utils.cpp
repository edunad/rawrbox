#include <rawrbox/steamworks/utils.hpp>
#include <rawrbox/utils/logger.hpp>

#include <steam/isteamutils.h>

namespace rawrbox {
	rawrbox::SteamImage SteamUTILS::getImage(int handle) {
		if (SteamUtils() == nullptr) throw rawrbox::Logger::err("SteamUTILS", "SteamUtils not initialized");
		if (handle == 0) throw rawrbox::Logger::err("SteamUTILS", "Invalid handle");

		rawrbox::SteamImage image = {};

		bool success = SteamUtils()->GetImageSize(handle, &image.width, &image.height);
		if (!success) return image;

		const uint32_t uImageSizeInBytes = image.width * image.height * 4U;
		image.pixels.resize(uImageSizeInBytes);

		SteamUtils()->GetImageRGBA(handle, image.pixels.data(), uImageSizeInBytes);
		return image;
	}
} // namespace rawrbox
