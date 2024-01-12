#pragma once

#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/textures/flat.hpp>
#include <rawrbox/render/textures/missing.hpp>
#include <rawrbox/render/window.hpp>

#include <Shader.h>

#include <memory>

// NOLINTBEGIN(*)
#define RAWRBOX_DESTROY(HANDLE) \
	if (HANDLE != nullptr) { \
		HANDLE.Release(); \
	}
// NOLINTEND(*)

namespace rawrbox {
	class CameraBase;

	constexpr auto MAX_BONES_PER_VERTEX = 4;
	constexpr auto MAX_BONES_PER_MODEL = 200;

	// QUICK ACCESS ---
	extern uint32_t FRAME;

	extern Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SHADER_FACTORY;

	extern rawrbox::RendererBase* RENDERER;
	extern rawrbox::CameraBase* MAIN_CAMERA;
	// -----------

	// TEXTURE FALLBACKS ---
	extern std::shared_ptr<rawrbox::TextureMissing> MISSING_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureMissing> MISSING_VERTEX_TEXTURE;

	extern std::shared_ptr<rawrbox::TextureFlat> WHITE_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> BLACK_TEXTURE;
	extern std::shared_ptr<rawrbox::TextureFlat> NORMAL_TEXTURE;
	// ----

	// FONT FALLBACKS ----
	extern rawrbox::Font* DEBUG_FONT_REGULAR;
	extern rawrbox::Font* DEBUG_FONT_BOLD;
	extern rawrbox::Font* DEBUG_FONT_ITALIC;
	// -------------------

	// ID GENERATION
	extern size_t SOUND_ID;
	extern size_t LIGHT_ID;
	extern size_t EMITTER_ID;
	extern size_t TEXT_ID;
	// -----

	// OTHER INTERNAL
	extern bool __LIGHT_DIRTY__;
	extern bool __DECALS_DIRTY__;
	// --------------
} // namespace rawrbox
