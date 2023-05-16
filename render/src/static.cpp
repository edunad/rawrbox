#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	bgfx::ViewId CURRENT_VIEW_ID = 0;
	bool BGFX_INITIALIZED = false;

	// TEXTURE FALLBACKS ---
	std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE = nullptr;
	// ---------

	// ID GENERATION
	size_t SOUND_ID = 0;
	size_t LIGHT_ID = 0;
	size_t EMITTER_ID = 0;
	// -----
} // namespace rawrbox
