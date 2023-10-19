#pragma once
#include <rawrbox/render/texture/image.hpp>

namespace rawrbox {
	class TextureAtlas : public rawrbox::TextureImage {
	protected:
		std::vector<std::vector<uint8_t>> _tiles = {};

		uint16_t _spriteSize = 32;
		void processAtlas();

	public:
		explicit TextureAtlas(const std::filesystem::path& filePath, uint16_t spriteSize = 32, bool useFallback = true);
		explicit TextureAtlas(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, uint16_t spriteSize = 32, bool useFallback = true);

		TextureAtlas(const TextureAtlas&) = default;
		TextureAtlas(TextureAtlas&&) = delete;
		TextureAtlas& operator=(const TextureAtlas&) = delete;
		TextureAtlas& operator=(TextureAtlas&&) = delete;
		~TextureAtlas() override = default;

		// ------ PIXEL-UTILS
		[[nodiscard]] size_t total() const;
		[[nodiscard]] uint16_t getSpriteSize() const;
		[[nodiscard]] std::vector<uint8_t> getSprite(size_t id) const;
		// --------------------

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;
	};
} // namespace rawrbox
