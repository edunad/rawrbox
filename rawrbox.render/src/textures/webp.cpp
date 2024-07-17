
#include <rawrbox/render/textures/utils/webp.hpp>
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/utils/file.hpp>

#include <fmt/format.h>
namespace rawrbox {
	TextureWEBP::TextureWEBP(const std::filesystem::path& filePath, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, useFallback) { this->internalLoad(rawrbox::FileUtils::getRawData(this->_filePath), useFallback); }
	TextureWEBP::TextureWEBP(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, buffer, useFallback) { this->internalLoad(buffer, useFallback); }

	void TextureWEBP::internalLoad(const std::vector<uint8_t>& data, bool useFallback) {
		this->_name = "RawrBox::Texture::WEBP";

		try {
			this->_data = rawrbox::WEBP::decode(data);
		} catch (const std::runtime_error& err) {
			if (useFallback) {
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), err.what());
				this->loadFallback();
				return;
			}

			throw err;
		}
	}
} // namespace rawrbox
