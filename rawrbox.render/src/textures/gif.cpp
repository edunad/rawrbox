
#include <rawrbox/render/textures/gif.hpp>
#include <rawrbox/render/textures/utils/gif.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureGIF::TextureGIF(const std::filesystem::path& filePath, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, useFallback) { this->internalLoad({}, useFallback); }
	TextureGIF::TextureGIF(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, buffer, useFallback) { this->internalLoad(buffer, useFallback); }

	void TextureGIF::internalLoad(const std::vector<uint8_t>& buffer, bool useFallback) {
		this->_name = "RawrBox::Texture::GIF";

		try {
			if (buffer.empty()) {
				this->_data = rawrbox::GIF::decode(this->_filePath);
			} else {
				this->_data = rawrbox::GIF::decode(buffer);
			}
#ifdef RAWRBOX_TRACE_EXCEPTIONS
		} catch (const cpptrace::exception_with_message& e) {
#else
		} catch (const std::exception& e) {
#endif
			if (useFallback) {
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), e.what());
				this->loadFallback();
				return;
			}

			throw e;
		}
	}
} // namespace rawrbox
