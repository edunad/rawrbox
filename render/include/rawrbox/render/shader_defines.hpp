#pragma once
#include <bgfx/embedded_shader.h>

// NOLINTBEGIN(*)
// Disable dx9
#undef BGFX_PLATFORM_SUPPORTS_DX9BC
#define BGFX_PLATFORM_SUPPORTS_DX9BC 0
#undef BGFX_EMBEDDED_SHADER_DX9BC
#define BGFX_EMBEDDED_SHADER_DX9BC(...)
// ----

// Disable glsl (Old opengl)
#undef BGFX_PLATFORM_SUPPORTS_GLSL
#define BGFX_PLATFORM_SUPPORTS_GLSL 0
#undef BGFX_EMBEDDED_SHADER_GLSL
#define BGFX_EMBEDDED_SHADER_GLSL(...)
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
