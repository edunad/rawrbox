#include <rawrbox/render/textures/image.hpp>
#include <rawrbox/render/textures/utils/stbi.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureImage::TextureImage(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : _filePath(filePath) {
		try {
			this->_data = rawrbox::STBI::decode(buffer);
			if (!this->_data.valid() || this->_data.total() == 0) throw this->_logger->error("Invalid image data!");
#ifdef RAWRBOX_TRACE_EXCEPTIONS
		} catch (const cpptrace::exception_with_message& e) {
#else
		} catch (const std::exception& e) {
#endif
			if (useFallback) {
				this->loadFallback();
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), e.what());
				return;
			}

			throw e;
		}
	}

	TextureImage::TextureImage(const std::filesystem::path& filePath, bool useFallback) : _filePath(filePath) {
		try {
			this->_data = rawrbox::STBI::decode(filePath);
			if (!this->_data.valid() || this->_data.total() == 0) throw this->_logger->error("Invalid image data!");
#ifdef RAWRBOX_TRACE_EXCEPTIONS
		} catch (const cpptrace::exception_with_message& e) {
#else
		} catch (const std::exception& e) {
#endif
			if (useFallback) {
				this->loadFallback();
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), e.what());
				return;
			}

			throw e;
		}
	}

	TextureImage::TextureImage(const uint8_t* buffer, int bufferSize, bool useFallback) {
		try {
			this->_data = rawrbox::STBI::decode(buffer, bufferSize);
			if (!this->_data.valid() || this->_data.total() == 0) throw this->_logger->error("Invalid image data!");
#ifdef RAWRBOX_TRACE_EXCEPTIONS
		} catch (const cpptrace::exception_with_message& e) {
#else
		} catch (const std::exception& e) {
#endif
			if (useFallback) {
				this->loadFallback();
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), e.what());
				return;
			}

			throw e;
		}
	}

	TextureImage::TextureImage(const rawrbox::Vector2u& size, const uint8_t* buffer, uint8_t channels) {
		this->_data.size = size;
		this->_data.channels = channels;

		rawrbox::ImageFrame frame = {};

		frame.pixels.resize(static_cast<uint32_t>(size.x * size.y * channels));
		std::memcpy(frame.pixels.data(), buffer, static_cast<uint32_t>(frame.pixels.size()) * sizeof(uint8_t));

		// Check for transparency ----
		if (channels == 4U) {
			for (size_t i = 0; i < frame.pixels.size(); i += channels) {
				if (frame.pixels[i + 3] == 1.F) continue;
				this->_transparent = true;
				break;
			}
		}
		// ---------------------------

		this->_data.frames.emplace_back(frame);
	}

	TextureImage::TextureImage(const rawrbox::Vector2u& size, const std::vector<uint8_t>& buffer, uint8_t channels) : rawrbox::TextureImage(size, buffer.data(), channels) {}
	TextureImage::TextureImage(const rawrbox::Vector2u& size, uint8_t channels) {
		this->_data.size = size;
		this->_data.channels = channels;
		this->_data.createFrame();
	}
	// NOLINTEND(modernize-pass-by-value)
} // namespace rawrbox
