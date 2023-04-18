#pragma once
#include <bgfx/bgfx.h>

#define RAWRBOX_DESTROY(HANDLE) \
	if (bgfx::isValid(HANDLE)) { \
		bgfx::destroy(HANDLE); \
		HANDLE = BGFX_INVALID_HANDLE; \
	}

namespace rawrBox {
	extern bgfx::ViewId CURRENT_VIEW_ID;
} // namespace rawrBox
