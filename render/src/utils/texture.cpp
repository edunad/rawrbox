
#include <rawrbox/render/utils/texture.hpp>

namespace rawrbox {
	rawrbox::Vector4f TextureUtils::atlasToUV(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, size_t id) {
		// UV -------
		rawrbox::Vector2i totalSprites = atlasSize / spriteSize;
		rawrbox::Vector2f spriteSizeInUV = {static_cast<float>(spriteSize) / atlasSize.x, static_cast<float>(spriteSize) / atlasSize.y};

		uint32_t spriteId = std::clamp<uint32_t>(id, 0, (totalSprites.x * totalSprites.y));

		auto Y = static_cast<uint32_t>(std::floor(spriteId / totalSprites.x));
		auto X = spriteId - Y * totalSprites.x;

		rawrbox::Vector2f uvS = spriteSizeInUV * Vector2f(static_cast<float>(X), static_cast<float>(Y));
		rawrbox::Vector2f uvE = uvS + spriteSizeInUV;

		// -----------
		return {uvS.x, uvS.y, uvE.x, uvE.y};
	};

	rawrbox::Vector4f TextureUtils::atlasToShader(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, size_t id) {
		rawrbox::Vector2i totalSprites = atlasSize / spriteSize;

		uint32_t spriteId = std::clamp<uint32_t>(id, 0, (totalSprites.x * totalSprites.y));
		rawrbox::Vector2f spriteSizeInUV = {static_cast<float>(spriteSize) / atlasSize.x, static_cast<float>(spriteSize) / atlasSize.y};

		auto Y = static_cast<uint32_t>(std::floor(spriteId / totalSprites.x));
		auto X = spriteId - Y * totalSprites.x;

		rawrbox::Vector2f uvS = spriteSizeInUV * Vector2f(static_cast<float>(X), static_cast<float>(Y));

		return {uvS.x, uvS.y, static_cast<float>(totalSprites.x) / 2, 0}; // Because bgfx shaders only like floats :p
	}

} // namespace rawrbox
