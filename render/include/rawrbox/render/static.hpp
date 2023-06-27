#pragma once
#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/texture/flat.hpp>
#include <rawrbox/render/texture/image.hpp>
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
	extern uint32_t BGFX_FRAME;

	// VIEW IDS ---
	extern bgfx::ViewId GBUFFER_COLOR_VIEW_ID;
	extern bgfx::ViewId GBUFFER_L_DEPTH_VIEW_ID;
	extern bgfx::ViewId GBUFFER_SHADOW_VIEW_ID;
	extern bgfx::ViewId GBUFFER_COMBINE_VIEW_ID;

	extern bgfx::ViewId STENCIL_VIEW_ID;

	extern bgfx::ViewId POST_PROCESSING_ID;
	extern bgfx::ViewId RENDERER_VIEW_ID;

	extern bgfx::ViewId CURRENT_VIEW_ID;
	// ---------

	// TEXTURE FALLBACKS ---
	extern std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> BLACK_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> NORMAL_TEXTURE;
	// ----

	// ID GENERATION
	extern size_t SOUND_ID;
	extern size_t LIGHT_ID;
	extern size_t EMITTER_ID;
	// -----

	// OTHER ---
	extern rawrbox::CameraBase* MAIN_CAMERA;
	// ----

	// INTERNAL, DO NOT USE
	extern size_t __OPEN_WINDOWS__;
	// -------
} // namespace rawrbox
