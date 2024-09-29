
#include <rawrbox/render/textures/utils/webp.hpp>
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/utils/file.hpp>

#include <fmt/format.h>
namespace rawrbox {
	TextureWEBP::TextureWEBP(const std::filesystem::path& filePath, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, useFallback) { this->internalLoad(rawrbox::FileUtils::getRawData(this->_filePath), useFallback); }
	TextureWEBP::TextureWEBP(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, buffer, useFallback) { this->internalLoad(buffer, useFallback); }
	TextureWEBP::TextureWEBP(const std::filesystem::path& filePath, const uint8_t* buffer, size_t bufferSize, bool useFallback) : TextureAnimatedBase(filePath, useFallback) { this->internalLoad(buffer, bufferSize, useFallback); }

	void TextureWEBP::internalLoad(const uint8_t* buffer, size_t bufferSize, bool useFallback) {
		this->_name = "RawrBox::Texture::WEBP";

		try {
			this->_data = rawrbox::WEBP::decode(buffer, bufferSize);
#ifdef RAWRBOX_TRACE_EXCEPTIONS
		} catch (const cpptrace::exception_with_message& e) {
#else
		} catch (const std::exception& e) {
#endif
			if (useFallback) {
				this->_logger->warn("Failed to load '{}' ──> \n\t{}\n\t\t  └── Loading fallback texture!", this->_filePath.generic_string(), e.what());
				this->loadFallback();
				return;
			}

			throw e;
		}
	}

	void TextureWEBP::internalLoad(const std::vector<uint8_t>& data, bool useFallback) {
		this->internalLoad(data.data(), data.size(), useFallback);
	}

} // namespace rawrbox
