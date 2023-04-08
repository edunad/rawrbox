#pragma once
#include <bgfx/embedded_shader.h>

// Fix BGFX defines for linux
#undef BGFX_PLATFORM_SUPPORTS_DX9BC
#define BGFX_PLATFORM_SUPPORTS_DX9BC (0 \
	|| BX_PLATFORM_WINDOWS              \
	)

#undef BGFX_PLATFORM_SUPPORTS_DXBC
#define BGFX_PLATFORM_SUPPORTS_DXBC (0  \
	|| BX_PLATFORM_WINDOWS              \
	|| BX_PLATFORM_WINRT                \
	|| BX_PLATFORM_XBOXONE              \
	)
// ----------------
