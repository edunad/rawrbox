#pragma once
#include <bgfx/embedded_shader.h>

// Fix BGFX defines for linux
#ifndef _WIN32
	#define BGFX_PLATFORM_SUPPORTS_DX9BC 0
	#define BGFX_PLATFORM_SUPPORTS_DXBC 0

	#undef BGFX_EMBEDDED_SHADER_DX9BC
	#undef BGFX_EMBEDDED_SHADER_DXBC
#endif
// ----------------
