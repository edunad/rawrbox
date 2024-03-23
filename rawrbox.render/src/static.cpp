
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// QUICK ACCESS ---
	uint64_t FRAME = 0;
	Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SHADER_FACTORY;

	rawrbox::RendererBase* RENDERER = nullptr;
	rawrbox::CameraBase* MAIN_CAMERA = nullptr;
	// -----------

	// TEXTURE FALLBACKS ---
	std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureMissing> MISSING_VERTEX_TEXTURE = nullptr;

	std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> BLACK_TEXTURE = nullptr;
	std::shared_ptr<rawrbox::TextureFlat> NORMAL_TEXTURE = nullptr;
	// ---------

	// FONT FALLBACKS ----
	rawrbox::Font* DEBUG_FONT_REGULAR = nullptr;
	rawrbox::Font* DEBUG_FONT_BOLD = nullptr;
	rawrbox::Font* DEBUG_FONT_ITALIC = nullptr;
	// -------------------

	// ID GENERATION  -----
	size_t SOUND_ID = 0;
	size_t LIGHT_ID = 0;
	size_t EMITTER_ID = 0;
	size_t TEXT_ID = 0;
	// -----

	// OTHER INTERNAL  -----
	bool __LIGHT_DIRTY__ = true;
	bool __DECALS_DIRTY__ = true;
	// --------------
} // namespace rawrbox
