#pragma once

#include <rawrbox/render/textures/base.hpp>

#include <filesystem>

namespace rawrbox {
	class TextureAtlas : public rawrbox::TextureBase {
	protected:
		uint32_t _spriteSize = 32;

		void processAtlas(const rawrbox::ImageData& data);

	public:
		explicit TextureAtlas(const std::filesystem::path& filePath, uint32_t spriteSize = 32, bool useFallback = true);
		explicit TextureAtlas(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, uint32_t spriteSize = 32, bool useFallback = true);

		TextureAtlas(const TextureAtlas&) = delete;
		TextureAtlas(TextureAtlas&&) = delete;
		TextureAtlas& operator=(const TextureAtlas&) = delete;
		TextureAtlas& operator=(TextureAtlas&&) = delete;
		~TextureAtlas() override = default;

		// ------ PIXEL-UTILS
		[[nodiscard]] size_t total() const;
		[[nodiscard]] uint32_t getSpriteSize() const;
		[[nodiscard]] std::vector<uint8_t> getSprite(size_t id) const;
		// --------------------
	};
} // namespace rawrbox
