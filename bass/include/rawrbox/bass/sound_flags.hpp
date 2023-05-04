#pragma once

#include <cstdint>

namespace rawrBox {
	namespace SoundFlags {
		const uint32_t NONE = 0;
		const uint32_t SOUND_3D = 1 << 1;
		const uint32_t BEAT_DETECTION = 1 << 2;
		const uint32_t NO_BLOCK = 1 << 3;
	}; // namespace SoundFlags
} // namespace rawrBox
