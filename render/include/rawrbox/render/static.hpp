#pragma once

#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/texture/flat.hpp>
#include <rawrbox/render/texture/missing.hpp>

#include <Common/interface/RefCntAutoPtr.hpp>

#include <Graphics\GraphicsEngine\interface\Shader.h>

#include <memory>

// NOLINTBEGIN(*)
#define RAWRBOX_DESTROY(HANDLE) \
	if (HANDLE != nullptr) { \
		HANDLE.Release(); \
	}
// NOLINTEND(*)

namespace rawrbox {
	class RendererBase;

	enum RENDER_DEBUG_MODE {
		DEBUG_OFF,
		DEBUG_CLUSTER_Z,
		DEBUG_CLUSTER_COUNT,
		DEBUG_DECALS
	};

	constexpr auto MAX_BONES_PER_VERTEX = 4;
	constexpr auto MAX_BONES_PER_MODEL = 200;

	extern bool ENGINE_INITIALIZED;

	// GPU PICKING ---
	constexpr size_t GPU_PICK_SAMPLE_SIZE = 8 * 8 * 4;
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
	extern uint32_t FRAME;
	extern rawrbox::CameraBase* MAIN_CAMERA;
	extern rawrbox::RendererBase* RENDERER;
	extern rawrbox::Matrix4x4 TRANSFORM;
	extern bool MTX_RIGHT_HANDED;

	extern Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SHADER_FACTORY;
	//
	// extern rawrbox::RENDER_DEBUG_MODE RENDERER_DEBUG;
	// ----

	// INTERNAL, DO NOT USE
	extern size_t __OPEN_WINDOWS__;
	extern bool __LIGHT_DIRTY__;
	// -------
} // namespace rawrbox
