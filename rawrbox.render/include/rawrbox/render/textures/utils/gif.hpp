#pragma once

#include <rawrbox/utils/logger.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace rawrbox {
	struct ImageData;
	class GIF {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

		static rawrbox::ImageData internalLoad(int width, int height, int frames_n, uint8_t* gifPixels, int* delays);

	public:
		static rawrbox::ImageData decode(const std::filesystem::path& path);
		static rawrbox::ImageData decode(const std::vector<uint8_t>& data);
	};
} // namespace rawrbox
