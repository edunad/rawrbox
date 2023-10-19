#pragma once

#include <Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#include <vector>

namespace rawrbox {
	class TopologyUtils {
	public:
		// ---
		static uint32_t triToLine(void* dest, uint32_t destSize, const void* indc, uint32_t indcSize, bool _index32);
	};
} // namespace rawrbox
