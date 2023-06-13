#pragma once
#include <rawrbox/render/texture/flat.hpp>
#include <rawrbox/render/texture/missing.hpp>

#include <bgfx/bgfx.h>

#include <memory>

// NOLINTBEGIN(*)
#define RAWRBOX_DESTROY(HANDLE) \
	if (bgfx::isValid(HANDLE)) { \
		bgfx::destroy(HANDLE); \
		(HANDLE) = BGFX_INVALID_HANDLE; \
	}
// NOLINTEND(*)
namespace rawrbox {
	constexpr auto MAX_BONES_PER_VERTEX = 4;
	constexpr auto MAX_BONES_PER_MODEL = 200;
	constexpr auto MAX_LIGHTS = 12;

	extern bool BGFX_INITIALIZED;

	// VIEW IDS ---
	extern bgfx::ViewId RENDER_VIEW_ID;  // View 0 reserved for basic rendering
	extern bgfx::ViewId STENCIL_VIEW_ID; // View 1 reserved for stencil rendering
	extern bgfx::ViewId SHADOW_VIEW_ID;  // View 2 reserved for shadow rendering

	extern bgfx::ViewId POST_PROCESSING_ID; // 10 Views for post processing rendering
	extern bgfx::ViewId RENDERER_VIEW_ID;   // Rest of the views for textureRendering and etc

	extern bgfx::ViewId CURRENT_VIEW_ID;
	// ---------

	// TEXTURE FALLBACKS ---
	extern std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> BLACK_TEXTURE;
	// ----

	// ID GENERATION
	extern size_t SOUND_ID;
	extern size_t LIGHT_ID;
	extern size_t EMITTER_ID;
	// -----

	// INTERNAL, DO NOT USE
	extern size_t __OPEN_WINDOWS__;
	// -------
} // namespace rawrbox
