#pragma once

#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/texture/flat.hpp>
#include <rawrbox/render/texture/missing.hpp>
#include <rawrbox/render/window.hpp>

#include <memory>

// NOLINTBEGIN(*)
#define RAWRBOX_DESTROY(HANDLE) \
	if (HANDLE != nullptr) { \
		HANDLE.Release(); \
	}
// NOLINTEND(*)

namespace rawrbox::render {

	constexpr auto MAX_BONES_PER_VERTEX = 4;
	constexpr auto MAX_BONES_PER_MODEL = 200;

	// ENGINE ----
	// INTERNAL ----------------
	extern std::vector<std::unique_ptr<rawrbox::Window>> __WINDOWS;
	extern Diligent::RENDER_DEVICE_TYPE __RENDER_TYPE;
	extern std::unique_ptr<rawrbox::RendererBase> __RENDERER;
	// -------------------------

	extern rawrbox::RendererBase* RENDERER;

	extern rawrbox::Window* createWindow(Diligent::RENDER_DEVICE_TYPE render = Diligent::RENDER_DEVICE_TYPE_UNDEFINED);
	extern rawrbox::Window* getWindow(size_t indx = 0);
	extern void pollEvents();
	extern void shutdown();
	extern void update();
	extern void render();

	template <class T = RendererBase, typename... CallbackArgs>
	T* createRenderer(rawrbox::Window* window, CallbackArgs&&... args) {
		__RENDERER = std::make_unique<T>(__RENDER_TYPE, window->getHandle(), window->getSize(), std::forward<CallbackArgs>(args)...);
		RENDERER = __RENDERER.get();

		// Setup resize ----
		window->onResize += [](auto&, auto& size) {
			RENDERER->resize(size);
		};
		// -----------------

		return RENDERER;
	};
	// -----------

	// QUICK ACCESS ---
	extern uint32_t FRAME;
	extern bool ENGINE_INITIALIZED;
	extern Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SHADER_FACTORY;
	extern rawrbox::Matrix4x4 TRANSFORM;
	// -----------

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

	// OTHER INTERNAL
	extern bool __LIGHT_DIRTY__;
	// --------------
} // namespace rawrbox::render
