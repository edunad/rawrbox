
#include <rawrbox/render/textures/gif.hpp>

#include <stb/gif.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureGIF::TextureGIF(const std::filesystem::path& filePath, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, useFallback) { this->internalLoad({}, useFallback); }
	TextureGIF::TextureGIF(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, buffer, useFallback) { this->internalLoad(buffer, useFallback); }

	void TextureGIF::internalLoad(const std::vector<uint8_t>& buffer, bool useFallback) {
		this->_frames.clear();
		this->_channels = 4; // Force 4 channels on GIFS
		this->_name = "RawrBox::Texture::GIF";

		int frames_n = 0;
		int* delays = nullptr;
		uint8_t* gifPixels = nullptr;

		int width = 0;
		int height = 0;

		// Need to find a way to not load it all to memory
		if (buffer.empty()) {
			gifPixels = stbi_xload_file(this->_filePath.generic_string().c_str(), &width, &height, &frames_n, &delays);
		} else {
			gifPixels = stbi_xload_mem(buffer.data(), static_cast<int>(buffer.size()), &width, &height, &frames_n, &delays);
		}

		if (gifPixels == nullptr || delays == nullptr) {
			const auto* failure = stbi_failure_reason();

			if (useFallback) {
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), failure);
				this->loadFallback();
				return;
			}

			throw this->_logger->error("Error loading image: {}", failure);
		}

		this->_size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

		uint32_t framePixelCount = this->_size.x * this->_size.y * this->_channels;
		for (int i = 0; i < frames_n; i++) {
			// NOLINTBEGIN(*)
			this->_frames.push_back({}); // first push it, then allocate to prevent double copy of memory
			// NOLINTEND(*)

			rawrbox::Frame& frame = this->_frames.back();
			frame.delay = static_cast<float>(delays[i]); // in ms
			frame.pixels.resize(framePixelCount);

			auto* pixelsOffset = gifPixels + i * framePixelCount;
			std::copy(pixelsOffset, pixelsOffset + framePixelCount, frame.pixels.data());
		}

		// Check for transparency ----
		if (this->_channels == 4 && !this->_frames.empty()) {
			auto& frame1 = this->_frames.back().pixels;
			for (size_t i = 0; i < frame1.size(); i += this->_channels) {
				if (frame1[i + 3] == 1.F) continue;
				_transparent = true;
				break;
			}
		}
		// ---------------------------

		stbi_image_free(gifPixels);
		stbi_image_free(delays);

		// Set first frame ---
		if (!this->_frames.empty()) {
			this->_pixels = this->_frames[0].pixels;
		}
	}
} // namespace rawrbox
