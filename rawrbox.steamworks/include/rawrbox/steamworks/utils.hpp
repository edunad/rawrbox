#pragma once

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
	public:
		static rawrbox::SteamImage getImage(int handle);
	};
} // namespace rawrbox
