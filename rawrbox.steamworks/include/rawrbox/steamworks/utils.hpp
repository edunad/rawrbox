#pragma once

#include <rawrbox/utils/logger.hpp>

#include <cstdint>
#include <vector>

namespace rawrbox {
	struct SteamImage {
	public:
		std::vector<uint8_t> pixels = {};

		uint32_t width = 0;
		uint32_t height = 0;
	};

	class SteamUTILS {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static rawrbox::SteamImage getImage(int handle);
	};
} // namespace rawrbox
