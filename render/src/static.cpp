
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// VIEW IDS ---
	bgfx::ViewId MAIN_WORLD_VIEW = 100;
	bgfx::ViewId MAIN_OVERLAY_VIEW = 101;

	bgfx::ViewId STENCIL_VIEW_ID = 110; // 10 Views reserved for stencil rendering

	bgfx::ViewId POST_PROCESSING_ID = 120; // 10 Views for post processing rendering
	bgfx::ViewId RENDERER_VIEW_ID = 130;   // Rest of the views for textureRendering and etc

	bgfx::ViewId CURRENT_VIEW_ID = MAIN_WORLD_VIEW; // Default -
	// ---------

	bool BGFX_INITIALIZED = false;
	uint32_t BGFX_FRAME = 0;

	// TEXTURE FALLBACKS ---
	std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> BLACK_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> NORMAL_TEXTURE = nullptr;
	// ---------

	// GLOBAL UNIFORMS ---
	uint8_t SAMPLE_MAT_ALBEDO = 0;
	uint8_t SAMPLE_MAT_NORMAL = 1;
	uint8_t SAMPLE_MAT_SPECULAR = 2;
	uint8_t SAMPLE_MAT_EMISSION = 3;
	uint8_t SAMPLE_MAT_OPACITY = 4;
	uint8_t SAMPLE_MAT_DISPLACEMENT = 5;

	uint8_t SAMPLE_LIGHTS = 6;

	uint8_t SAMPLE_CLUSTERS = 7;
	uint8_t SAMPLE_LIGHTINDICES = 8;
	uint8_t SAMPLE_LIGHTGRID = 9;
	uint8_t SAMPLE_ATOMIC_INDEX = 10;

	uint8_t SAMPLE_INSTANCE_DATA = 11;
	// ---------

	// ID GENERATION
	size_t SOUND_ID = 0;
	size_t LIGHT_ID = 0;
	size_t EMITTER_ID = 0;
	// -----

	// QUICK ACCESS ---
	rawrbox::CameraBase* MAIN_CAMERA = nullptr;
	rawrbox::RendererBase* RENDERER = nullptr;

	rawrbox::RENDER_DEBUG_MODE RENDERER_DEBUG = rawrbox::RENDER_DEBUG_MODE::DEBUG_OFF;
	// ----

	// INTERNAL, DO NOT USE
	size_t __OPEN_WINDOWS__ = 0;
	bool __LIGHT_DIRTY__ = false;
	// -------
} // namespace rawrbox
