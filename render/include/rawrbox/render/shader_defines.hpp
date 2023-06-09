#pragma once
#include <bgfx/embedded_shader.h>

// NOLINTBEGIN(*)
// Disable dx9
#undef BGFX_PLATFORM_SUPPORTS_DX9BC
#define BGFX_PLATFORM_SUPPORTS_DX9BC 0
#undef BGFX_EMBEDDED_SHADER_DX9BC
#define BGFX_EMBEDDED_SHADER_DX9BC(...)
// ----

// Fix BGFX defines for linux
#ifndef _WIN32
	#undef BGFX_PLATFORM_SUPPORTS_DXBC
	#define BGFX_PLATFORM_SUPPORTS_DXBC 0
	#undef BGFX_EMBEDDED_SHADER_DXBC
	#define BGFX_EMBEDDED_SHADER_DXBC(...)
#endif
// ----------------
// NOLINTEND(*)

#include <generated/shaders/render/all.hpp>
