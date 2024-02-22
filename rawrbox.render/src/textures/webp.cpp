
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/utils/path.hpp>

#include <fmt/format.h>
#include <webp/decode.h>
#include <webp/demux.h>

namespace rawrbox {
	TextureWEBP::TextureWEBP(const std::filesystem::path& filePath, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, useFallback) { this->internalLoad(rawrbox::PathUtils::getRawData(this->_filePath), useFallback); }
	TextureWEBP::TextureWEBP(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, buffer, useFallback) { this->internalLoad(buffer, useFallback); }

	void TextureWEBP::internalLoad(const std::vector<uint8_t>& buffer, bool useFallback) {
		this->_name = "RawrBox::Texture::WEBP";

		try {
			if (buffer.empty()) throw this->_logger->error("Image not found");
			WebPData webp_data = {buffer.data(), buffer.size()};

			WebPAnimDecoderOptions options;
			if (WebPAnimDecoderOptionsInit(&options) == 0) {
				throw this->_logger->error("Error initializing image '{}'!", this->_filePath.generic_string());
			}

			options.use_threads = 1;
			auto* decoder = WebPAnimDecoderNew(&webp_data, &options);
			if (decoder == nullptr) {
				throw this->_logger->error("Error initializing decoder '{}'!", this->_filePath.generic_string());
			}

			WebPAnimInfo info;
			if (WebPAnimDecoderGetInfo(decoder, &info) == 0) {
				WebPAnimDecoderDelete(decoder);
				throw this->_logger->error("Error decoding image info '{}'!", this->_filePath.generic_string());
			}

			this->_frames.clear();
			this->_frames.reserve(info.frame_count);

			this->_size = {static_cast<int>(info.canvas_width), static_cast<int>(info.canvas_height)};
			this->_channels = 4;

			float prevTime = 0.F;
			while (WebPAnimDecoderHasMoreFrames(decoder) != 0) {
				uint8_t* buf = nullptr;
				int delay = 0;
				if (WebPAnimDecoderGetNext(decoder, &buf, &delay) == 0) {
					WebPAnimDecoderDelete(decoder);
					throw this->_logger->error("Error decoding image '{}'!", this->_filePath.generic_string());
				}

				Frame frame;
				frame.delay = static_cast<float>(delay) - prevTime;
				frame.pixels.resize(this->_size.x * this->_size.y * this->_channels);
				std::memcpy(frame.pixels.data(), buf, this->_size.x * this->_size.y * this->_channels);

				this->_frames.push_back(frame);
				prevTime = static_cast<float>(delay);
			}

			WebPAnimDecoderDelete(decoder);
		} catch (std::runtime_error err) {
			if (useFallback) {
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), err.what());
				this->loadFallback();
				return;
			}

			throw err;
		}
	}
} // namespace rawrbox
