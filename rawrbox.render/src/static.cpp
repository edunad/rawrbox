
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// QUICK ACCESS ---
	uint32_t FRAME = 0;

	bool ENGINE_INITIALIZED = false;

	Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SHADER_FACTORY;

	rawrbox::RendererBase* RENDERER = nullptr;
	rawrbox::CameraBase* MAIN_CAMERA = nullptr;
	// -----------

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
	size_t TEXT_ID = 0;
	// -----

	// OTHER INTERNAL
	bool __LIGHT_DIRTY__ = true;
	// --------------
} // namespace rawrbox
