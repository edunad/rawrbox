#pragma once

#include <rawrbox/utils/logger.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace rawrbox {
	struct ImageData;
	class STBI {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

		static rawrbox::ImageData internalLoad(int width, int height, int channels, uint8_t* pixels);

	public:
		static rawrbox::ImageData decode(const std::filesystem::path& path);
		static rawrbox::ImageData decode(const std::vector<uint8_t>& data);
		static rawrbox::ImageData decode(const uint8_t* buffer, int bufferSize);
	};
} // namespace rawrbox
