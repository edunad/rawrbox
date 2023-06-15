#pragma once

#include <rawrbox/math/vector4.hpp>

namespace rawrbox {

	class TextureUtils {
	public:
		// ---
		static rawrbox::Vector4f atlasToUV(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, size_t id);
		static rawrbox::Vector4f atlasToShader(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, size_t id);
	};
} // namespace rawrbox
