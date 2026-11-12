#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/utils/stbi.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <tracy/Tracy.hpp>

// NOLINTBEGIN(clang-diagnostic-unknown-pragmas)
#pragma warning(push)
#pragma warning(disable : 4505)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_FAILURE_USERMSG
#include <stb/stb_image.hpp>
#include <stb/stb_image_write.hpp>
#pragma warning(pop)
// NOLINTEND(clang-diagnostic-unknown-pragmas)

#include <fstream>

namespace rawrbox {
	std::unique_ptr<rawrbox::Logger> STBI::_logger = std::make_unique<rawrbox::Logger>("RawrBox-STBI");

	rawrbox::ImageData STBI::internalLoad(int width, int height, int channels, uint8_t* pixels) {
		if (pixels == nullptr) {
			STBI::_logger->warn("Failed to decode image: {}", stbi_failure_reason());
			return {};
		}

		if (width <= 0 || height <= 0 || channels <= 0) {
			STBI::_logger->warn("Invalid image {}x{} with {} channels", width, height, channels);
			stbi_image_free(pixels);
			return {};
		}

		rawrbox::ImageData imgData = {};
		imgData.channels = static_cast<uint8_t>(channels);
		imgData.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		rawrbox::ImageFrame frame = {};
		frame.pixels.resize(static_cast<size_t>(width) * height * channels);

		std::memcpy(frame.pixels.data(), pixels, frame.pixels.size() * sizeof(uint8_t));
		imgData.frames.emplace_back(frame);

		stbi_image_free(pixels);
		return imgData;
	}

	rawrbox::ImageData STBI::decode(const std::filesystem::path& path) {
		if (!rawrbox::VFS::contains(path)) RAWRBOX_CRITICAL("Could not find file {}", path.generic_string());
		return STBI::decode(rawrbox::VFS::read(path));
	}

	rawrbox::ImageData STBI::decode(const std::vector<uint8_t>& data) {
		return STBI::decode(data.data(), static_cast<int>(data.size()));
	}

	rawrbox::ImageData STBI::decode(const uint8_t* buffer, int bufferSize) {
		if (buffer == nullptr || bufferSize <= 0) RAWRBOX_CRITICAL("Invalid data, cannot be empty!");
		ZoneScopedN("STBI::decode");

		int width = 0;
		int height = 0;
		int channels = 0;

		uint8_t* image = stbi_load_from_memory(buffer, bufferSize, &width, &height, &channels, 0);
		return STBI::internalLoad(width, height, channels, image);
	}

	std::optional<rawrbox::ImageInfo> STBI::info(const std::filesystem::path& path) {
		if (!rawrbox::VFS::contains(path)) return std::nullopt;

		const auto data = rawrbox::VFS::read(path);
		return STBI::info(std::span<const uint8_t>(data));
	}

	std::optional<rawrbox::ImageInfo> STBI::info(std::span<const uint8_t> data) {
		if (data.empty()) return std::nullopt;

		int width = 0;
		int height = 0;
		int channels = 0;

		if (stbi_info_from_memory(data.data(), static_cast<int>(data.size()), &width, &height, &channels) == 0) return std::nullopt;
		if (width <= 0 || height <= 0 || channels <= 0) return std::nullopt;

		return rawrbox::ImageInfo{.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, .channels = static_cast<uint8_t>(channels)};
	}

	std::vector<uint8_t> STBI::encodePNG(const rawrbox::Vector2u& size, uint8_t channels, std::span<const uint8_t> pixels, uint64_t stride) {
		if (size.x == 0U || size.y == 0U || channels == 0U || channels > 4U) return {};

		const uint64_t row = static_cast<uint64_t>(size.x) * channels;
		const uint64_t pitch = stride == 0U ? row : stride;
		if (pitch < row || pixels.size() < pitch * (size.y - 1U) + row) return {};

		std::vector<uint8_t> png = {};
		const auto write = [](void* context, void* data, int length) {
			auto* out = static_cast<std::vector<uint8_t>*>(context);
			const auto* bytes = static_cast<const uint8_t*>(data);

			out->insert(out->end(), bytes, bytes + length);
		};

		if (stbi_write_png_to_func(write, &png, static_cast<int>(size.x), static_cast<int>(size.y), channels, pixels.data(), static_cast<int>(pitch)) == 0) return {};
		return png;
	}

	bool STBI::writePNG(const std::filesystem::path& path, const rawrbox::Vector2u& size, uint8_t channels, std::span<const uint8_t> pixels, uint64_t stride) {
		const auto png = STBI::encodePNG(size, channels, pixels, stride);
		if (png.empty()) return false;

		std::error_code error;
		if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path(), error);

		std::ofstream stream(path, std::ios::binary | std::ios::trunc);
		if (!stream.is_open()) return false;

		stream.write(std::bit_cast<const char*>(png.data()), static_cast<std::streamsize>(png.size()));
		return stream.good();
	}
} // namespace rawrbox