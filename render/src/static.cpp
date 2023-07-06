
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// VIEW IDS ---
	bgfx::ViewId MAIN_DEFAULT_VIEW = 200;
	bgfx::ViewId STENCIL_VIEW_ID = 199; // View 3 reserved for stencil rendering

	bgfx::ViewId POST_PROCESSING_ID = 180; // 10 Views for post processing rendering
	bgfx::ViewId RENDERER_VIEW_ID = 190;   // Rest of the views for textureRendering and etc

	bgfx::ViewId CURRENT_VIEW_ID = MAIN_DEFAULT_VIEW; // Default -
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

	uint8_t SAMPLE_LIGHTS_POINTLIGHTS = 3;

	uint8_t SAMPLE_CLUSTERS = 4;
	uint8_t SAMPLE_LIGHTINDICES = 5;
	uint8_t SAMPLE_LIGHTGRID = 6;
	uint8_t SAMPLE_ATOMIC_INDEX = 7;
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
	// -------
} // namespace rawrbox
