#include <rawrbox/render/static.hpp>

namespace rawrbox {
	bgfx::ViewId CURRENT_VIEW_ID = 0;
	bool BGFX_INITIALIZED = false;

	// TEXTURE FALLBACKS ---
	std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> MISSING_SPECULAR_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> MISSING_EMISSION_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE = nullptr;
	// ---------

	// ID GENERATION
	int SOUND_ID = 0;
	int LIGHT_ID = 0;
	int EMITTER_ID = 0;
	// -----
} // namespace rawrbox
