#pragma once

#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/texture/flat.hpp>
#include <rawrbox/render/texture/missing.hpp>

#include <Common/interface/RefCntAutoPtr.hpp>

#include <Graphics\GraphicsEngine\interface\Shader.h>

#include <memory>

// NOLINTBEGIN(*)
#define RAWRBOX_DESTROY(HANDLE) \
	if (HANDLE != nullptr) { \
		HANDLE->Release(); \
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

	// GLOBAL UNIFORMS ---
	extern uint8_t SAMPLE_MAT_ALBEDO;
	extern uint8_t SAMPLE_MAT_NORMAL;
	extern uint8_t SAMPLE_MAT_SPECULAR;
	extern uint8_t SAMPLE_MAT_EMISSION;
	extern uint8_t SAMPLE_MAT_DISPLACEMENT;

	extern uint8_t SAMPLE_LIGHTS;

	extern uint8_t SAMPLE_MASK;
	extern uint8_t SAMPLE_DEPTH;

	extern uint8_t SAMPLE_CLUSTERS;
	extern uint8_t SAMPLE_LIGHTINDICES;
	extern uint8_t SAMPLE_LIGHTGRID;
	extern uint8_t SAMPLE_ATOMIC_INDEX;

	extern uint8_t SAMPLE_INSTANCE_DATA;
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
	// extern uint32_t BGFX_FRAME;
	//
	extern rawrbox::CameraBase* MAIN_CAMERA;
	extern rawrbox::RendererBase* RENDERER;
	extern rawrbox::Matrix4x4 TRANSFORM;
	extern Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SHADER_FACTORY;
	//
	// extern rawrbox::RENDER_DEBUG_MODE RENDERER_DEBUG;
	// ----

	// INTERNAL, DO NOT USE
	extern size_t __OPEN_WINDOWS__;
	extern bool __LIGHT_DIRTY__;
	// -------
} // namespace rawrbox
