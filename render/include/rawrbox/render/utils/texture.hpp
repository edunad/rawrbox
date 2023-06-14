#pragma once

#include <rawrbox/math/vector4.hpp>

namespace rawrbox {

	class TextureUtils {
	public:
		// ---
		static rawrbox::Vector4f atlasUV(const rawrbox::Vector2i& atlasSize, uint32_t spriteSize, size_t id);
	};
} // namespace rawrbox
