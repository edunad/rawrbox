
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// VIEW IDS ---
	bgfx::ViewId MAIN_VIEW_ID = 0;    // View 0
	bgfx::ViewId STENCIL_VIEW_ID = 3; // View 3 reserved for stencil rendering

	bgfx::ViewId POST_PROCESSING_ID = 4; // 10 Views for post processing rendering
	bgfx::ViewId RENDERER_VIEW_ID = 14;  // Rest of the views for textureRendering and etc

	bgfx::ViewId CURRENT_VIEW_ID = MAIN_VIEW_ID; // Default -
	// ---------

	bool BGFX_INITIALIZED = false;
	uint32_t BGFX_FRAME = 0;

	// TEXTURE FALLBACKS ---
	std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> BLACK_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> NORMAL_TEXTURE = nullptr;
	// ---------

	// ID GENERATION
	size_t SOUND_ID = 0;
	size_t LIGHT_ID = 0;
	size_t EMITTER_ID = 0;
	// -----

	// OTHER ---
	rawrbox::CameraBase* MAIN_CAMERA = nullptr;
	// ----

	// INTERNAL, DO NOT USE
	size_t __OPEN_WINDOWS__ = 0;
	// -------
} // namespace rawrbox
