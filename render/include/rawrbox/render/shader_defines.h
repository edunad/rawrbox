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

#if BGFX_PLATFORM_SUPPORTS_DX9BC
#	undef  BGFX_EMBEDDED_SHADER_DX9BC
#	define BGFX_EMBEDDED_SHADER_DX9BC(_renderer, _name) \
	{ _renderer, BX_CONCATENATE(_name, _dx9 ), BX_COUNTOF(BX_CONCATENATE(_name, _dx9 ) ) },
#endif // BGFX_PLATFORM_SUPPORTS_DX9BC

#if BGFX_PLATFORM_SUPPORTS_DXBC
#	undef  BGFX_EMBEDDED_SHADER_DXBC
#	define BGFX_EMBEDDED_SHADER_DXBC(_renderer, _name) \
	{ _renderer, BX_CONCATENATE(_name, _dx11), BX_COUNTOF(BX_CONCATENATE(_name, _dx11) ) },
#endif // BGFX_PLATFORM_SUPPORTS_DXBC
