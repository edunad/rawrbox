#pragma once

#include <rawrbox/math/vector4.hpp>

#include <cstdint>

namespace rawrbox {

	class TextureUtils {
	public:
		// ---
		static rawrbox::Vector4f atlasToUV(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, uint32_t id);
	};
} // namespace rawrbox
