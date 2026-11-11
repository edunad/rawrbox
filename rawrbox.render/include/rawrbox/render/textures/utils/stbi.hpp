#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/utils/logger.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace rawrbox {
	struct ImageData;

	struct ImageInfo {
		rawrbox::Vector2u size = {};
		uint8_t channels = 0;
	};

	class STBI {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

		static rawrbox::ImageData internalLoad(int width, int height, int channels, uint8_t* pixels);

	public:
		static rawrbox::ImageData decode(const std::filesystem::path& path);
		static rawrbox::ImageData decode(const std::vector<uint8_t>& data);
		static rawrbox::ImageData decode(const uint8_t* buffer, int bufferSize);

		[[nodiscard]] static std::optional<rawrbox::ImageInfo> info(const std::filesystem::path& path);
		[[nodiscard]] static std::optional<rawrbox::ImageInfo> info(std::span<const uint8_t> data);

		[[nodiscard]] static std::vector<uint8_t> encodePNG(const rawrbox::Vector2u& size, uint8_t channels, std::span<const uint8_t> pixels, uint64_t stride = 0);
		static bool writePNG(const std::filesystem::path& path, const rawrbox::Vector2u& size, uint8_t channels, std::span<const uint8_t> pixels, uint64_t stride = 0);
	};
} // namespace rawrbox
