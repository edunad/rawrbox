#pragma once
#include <rawrbox/render/texture/flat.hpp>
#include <rawrbox/render/texture/missing.hpp>
#include <rawrbox/utils/ringbuffer.hpp>

#include <bgfx/bgfx.h>

#include <functional>
#include <memory>
#include <thread>

// NOLINTBEGIN(*)
#define RAWRBOX_DESTROY(HANDLE) \
	if (bgfx::isValid(HANDLE)) { \
		bgfx::destroy(HANDLE); \
		(HANDLE) = BGFX_INVALID_HANDLE; \
	}
// NOLINTEND(*)
namespace rawrbox {
	class LightManager;

	constexpr auto MAX_BONES_PER_VERTEX = 4;
	constexpr auto MAX_BONES_PER_MODEL = 200;
	constexpr auto MAX_LIGHTS = 12;

	extern bool BGFX_INITIALIZED;

	extern bgfx::ViewId CURRENT_VIEW_ID;

	// TEXTURE FALLBACKS ---
	extern std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> MISSING_SPECULAR_EMISSIVE_TEXTURE;
	// ----

	extern rawrbox::LightManager Lights;

	// ID GENERATION
	extern size_t SOUND_ID;
	extern size_t LIGHT_ID;
	extern size_t EMITTER_ID;
	// -----

	// INTERNAL, DO NOT USE
	extern size_t __OPEN_WINDOWS__;
	// -------
} // namespace rawrbox
