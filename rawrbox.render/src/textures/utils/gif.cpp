#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/utils/gif.hpp>

#include <stb/stb_gif.hpp>

namespace rawrbox {
	std::unique_ptr<rawrbox::Logger> GIF::_logger = std::make_unique<rawrbox::Logger>("RawrBox-GIF");

	rawrbox::ImageData GIF::internalLoad(int width, int height, int frames_n, uint8_t* gifPixels, int* delays) {
		if (width == 0 || height == 0) CRITICAL_RAWRBOX("Invalid image size: {}x{}", width, height);
		if (gifPixels == nullptr || delays == nullptr) {
			const auto* failure = stbi_failure_reason();
			CRITICAL_RAWRBOX("Error loading image: {}", failure);
		}

		rawrbox::ImageData imgData = {};
		imgData.channels = 4;
		imgData.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		uint32_t framePixelCount = width * height * 4;
		for (int i = 0; i < frames_n; i++) {
			// NOLINTBEGIN(*)
			imgData.frames.push_back({}); // first push it, then allocate to prevent double copy of memory
			// NOLINTEND(*)

			rawrbox::ImageFrame& frame = imgData.frames.back();
			frame.delay = static_cast<float>(delays[i]); // in ms
			frame.pixels.resize(framePixelCount);

			auto* pixelsOffset = gifPixels + i * framePixelCount;
			std::copy(pixelsOffset, pixelsOffset + framePixelCount, frame.pixels.data());
		}

		stbi_image_free(gifPixels);
		stbi_image_free(delays);

		return imgData;
	}

	rawrbox::ImageData GIF::decode(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) CRITICAL_RAWRBOX("Could not find file {}", path.generic_string());

		int frames_n = 0;
		int* delays = nullptr;
		uint8_t* gifPixels = nullptr;

		int width = 0;
		int height = 0;

		gifPixels = stbi_xload_file(path.generic_string().c_str(), &width, &height, &frames_n, &delays);
		return internalLoad(width, height, frames_n, gifPixels, delays);
	}

	rawrbox::ImageData GIF::decode(const std::vector<uint8_t>& data) {
		if (data.empty()) CRITICAL_RAWRBOX("Invalid data, cannot be empty!");

		int frames_n = 0;
		int* delays = nullptr;
		uint8_t* gifPixels = nullptr;

		int width = 0;
		int height = 0;

		gifPixels = stbi_xload_mem(data.data(), static_cast<int>(data.size()), &width, &height, &frames_n, &delays);
		return internalLoad(width, height, frames_n, gifPixels, delays);
	}
} // namespace rawrbox
