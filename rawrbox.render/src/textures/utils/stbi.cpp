#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/utils/stbi.hpp>

// NOLINTBEGIN(clang-diagnostic-unknown-pragmas)
#pragma warning(push)
#pragma warning(disable : 4505)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_FAILURE_USERMSG
#include <stb/stb_image.hpp>
#pragma warning(pop)
// NOLINTEND(clang-diagnostic-unknown-pragmas)

namespace rawrbox {
	std::unique_ptr<rawrbox::Logger> STBI::_logger = std::make_unique<rawrbox::Logger>("RawrBox-STBI");

	rawrbox::ImageData STBI::internalLoad(int width, int height, int channels, uint8_t* pixels) {
		if (width == 0 || height == 0) CRITICAL_RAWRBOX("Invalid image size: {}x{}", width, height);
		if (channels == 0) CRITICAL_RAWRBOX("Invalid image channels: {}", channels);
		if (pixels == nullptr) {
			const auto* failure = stbi_failure_reason();
			CRITICAL_RAWRBOX("Error loading image: {}", failure);
		}

		rawrbox::ImageData imgData = {};
		imgData.channels = static_cast<uint8_t>(channels);
		imgData.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		rawrbox::ImageFrame frame = {};
		frame.pixels.resize(width * height * channels);

		std::memcpy(frame.pixels.data(), pixels, static_cast<uint32_t>(frame.pixels.size()) * sizeof(uint8_t));
		imgData.frames.emplace_back(frame);

		stbi_image_free(pixels);
		return imgData;
	}

	rawrbox::ImageData STBI::decode(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) CRITICAL_RAWRBOX("Could not find file {}", path.generic_string());

		int width = 0;
		int height = 0;
		int channels = 0;

		uint8_t* image = stbi_load(path.generic_string().c_str(), &width, &height, &channels, 0);
		if (image == nullptr) return {};

		return internalLoad(width, height, channels, image);
	}

	rawrbox::ImageData STBI::decode(const std::vector<uint8_t>& data) {
		if (data.empty()) CRITICAL_RAWRBOX("Invalid data, cannot be empty!");

		int width = 0;
		int height = 0;
		int channels = 0;

		uint8_t* image = stbi_load_from_memory(data.data(), static_cast<int>(data.size()) * sizeof(uint8_t), &width, &height, &channels, 0);
		if (image == nullptr) return {};

		return internalLoad(width, height, channels, image);
	}

	rawrbox::ImageData STBI::decode(const uint8_t* buffer, int bufferSize) {
		int width = 0;
		int height = 0;
		int channels = 0;

		uint8_t* image = stbi_load_from_memory(buffer, bufferSize, &width, &height, &channels, 0);
		if (image == nullptr) return {};

		return internalLoad(width, height, channels, image);
	}
} // namespace rawrbox
