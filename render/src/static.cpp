
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	bool ENGINE_INITIALIZED = false;

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

	// QUICK ACCESS ---
	uint32_t FRAME = 0;
	rawrbox::CameraBase* MAIN_CAMERA = nullptr;
	rawrbox::RendererBase* RENDERER = nullptr;
	rawrbox::Matrix4x4 TRANSFORM = {};
	bool MTX_RIGHT_HANDED = false;

	Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SHADER_FACTORY = {};
	//
	// rawrbox::RENDER_DEBUG_MODE RENDERER_DEBUG = rawrbox::RENDER_DEBUG_MODE::DEBUG_OFF;
	// ----

	// INTERNAL, DO NOT USE
	size_t __OPEN_WINDOWS__ = 0;
	bool __LIGHT_DIRTY__ = true;
	// -------
} // namespace rawrbox
