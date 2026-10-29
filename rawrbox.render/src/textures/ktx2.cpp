#include <rawrbox/render/textures/ktx2.hpp>
#include <rawrbox/render/textures/utils/ktx2.hpp>
#include <rawrbox/utils/file.hpp>

#include <tracy/Tracy.hpp>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureKTX2::TextureKTX2(const std::filesystem::path& filePath, bool useFallback) : _filePath(filePath) {
		const auto data = rawrbox::FileUtils::getRawData(filePath);
		this->internalLoad(data.data(), data.size(), useFallback);
	}

	TextureKTX2::TextureKTX2(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : _filePath(filePath) { this->internalLoad(buffer.data(), buffer.size(), useFallback); }
	TextureKTX2::TextureKTX2(const std::filesystem::path& filePath, const uint8_t* buffer, size_t bufferSize, bool useFallback) : _filePath(filePath) { this->internalLoad(buffer, bufferSize, useFallback); }
	// NOLINTEND(modernize-pass-by-value)

	void TextureKTX2::internalLoad(const uint8_t* buffer, size_t bufferSize, bool useFallback) {
		ZoneScopedN("TextureKTX2::load");
		this->_name = "RawrBox::Texture::KTX2";

		try {
			this->_data = rawrbox::KTX2::decode(buffer, bufferSize);
			this->_transparent = this->_data.transparent();
		} catch (const std::exception& e) {
			if (useFallback) {
				this->_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), e.what());
				this->loadFallback();
				return;
			}

			throw e;
		}
	}
} // namespace rawrbox