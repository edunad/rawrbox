#pragma once
#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/renderers/base.hpp>
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
	enum RENDER_DEBUG_MODE {
		DEBUG_OFF,
		DEBUG_CLUSTER_Z,
		DEBUG_CLUSTER_COUNT
	};

	constexpr auto MAX_BONES_PER_VERTEX = 4;
	constexpr auto MAX_BONES_PER_MODEL = 200;
	constexpr auto MAX_LIGHTS = 1000;

	extern bool BGFX_INITIALIZED;
	extern uint32_t BGFX_FRAME;

	// VIEW IDS ---
	extern bgfx::ViewId MAIN_WORLD_VIEW;
	extern bgfx::ViewId MAIN_OVERLAY_VIEW;

	extern bgfx::ViewId STENCIL_VIEW_ID;

	extern bgfx::ViewId POST_PROCESSING_ID;
	extern bgfx::ViewId RENDERER_VIEW_ID;

	extern bgfx::ViewId CURRENT_VIEW_ID;
	// ---------

	// GLOBAL UNIFORMS ---
	extern uint8_t SAMPLE_MAT_ALBEDO;
	extern uint8_t SAMPLE_MAT_NORMAL;
	extern uint8_t SAMPLE_MAT_SPECULAR;
	extern uint8_t SAMPLE_MAT_EMISSION;
	extern uint8_t SAMPLE_MAT_OPACITY;
	extern uint8_t SAMPLE_MAT_DISPLACEMENT;

	extern uint8_t SAMPLE_LIGHTS_POINTLIGHTS;

	extern uint8_t SAMPLE_CLUSTERS;
	extern uint8_t SAMPLE_LIGHTINDICES;
	extern uint8_t SAMPLE_LIGHTGRID;
	extern uint8_t SAMPLE_ATOMIC_INDEX;

	extern uint8_t SAMPLE_INSTANCE_DATA;
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

	// QUICK ACCESS ---
	extern rawrbox::CameraBase* MAIN_CAMERA;
	extern rawrbox::RendererBase* RENDERER;

	extern rawrbox::RENDER_DEBUG_MODE RENDERER_DEBUG;
	// ----

	// INTERNAL, DO NOT USE
	extern size_t __OPEN_WINDOWS__;
	// -------
} // namespace rawrbox
