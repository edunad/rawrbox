#pragma once
#include <rawrbox/render/texture/image.hpp>

namespace rawrbox {
	class TextureAtlas : public rawrbox::TextureImage {
	protected:
		uint16_t _spriteSize = 32;
		void processAtlas();

	public:
		explicit TextureAtlas(const std::filesystem::path& filePath, uint16_t spriteSize = 32, int forceChannels = 0, bool useFallback = true);
		explicit TextureAtlas(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, uint16_t spriteSize = 32, int forceChannels = 0, bool useFallback = true);

		TextureAtlas(const TextureAtlas&) = default;
		TextureAtlas(TextureAtlas&&) = delete;
		TextureAtlas& operator=(const TextureAtlas&) = delete;
		TextureAtlas& operator=(TextureAtlas&&) = delete;
		~TextureAtlas() override = default;

		// ------ PIXEL-UTILS
		// --------------------

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count) override;
	};
} // namespace rawrbox
