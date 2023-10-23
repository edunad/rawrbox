
#include <rawrbox/render/static.hpp>

namespace rawrbox::render {

	// ENGINE ----
	std::vector<std::unique_ptr<rawrbox::Window>> __WINDOWS = {};
	Diligent::RENDER_DEVICE_TYPE __RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_UNDEFINED;
	std::unique_ptr<rawrbox::RendererBase> __RENDERER = nullptr;
	rawrbox::RendererBase* RENDERER = nullptr;
	std::unique_ptr<rawrbox::CameraBase> __CAMERA = nullptr;

	rawrbox::Window* createWindow(Diligent::RENDER_DEVICE_TYPE render) {
		if (render == Diligent::RENDER_DEVICE_TYPE_UNDEFINED) {
#if PLATFORM_LINUX
	#if VULKAN_SUPPORTED
			__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_VULKAN;
	#else
			__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_GL;
	#endif
#else
	#if D3D12_SUPPORTED
			__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_D3D12;
	#elif D3D11_SUPPORTED
			__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_D3D11;
	#elif VULKAN_SUPPORTED
			__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_VULKAN;
	#else
			__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_GL;
	#endif
#endif
		} else {
			__RENDER_TYPE = render;
		}

		if (render == Diligent::RENDER_DEVICE_TYPE_GL || render == Diligent::RENDER_DEVICE_TYPE_GLES) {
			rawrbox::Matrix4x4::MTX_RIGHT_HANDED = true;
		}

		__WINDOWS.push_back(std::make_unique<rawrbox::Window>(__RENDER_TYPE));
		return __WINDOWS.front().get();
	}

	rawrbox::Window* getWindow(size_t indx) {
		if (indx > __WINDOWS.size()) throw std::runtime_error(fmt::format("[RawrBox] Invalid window index '{}'", indx));
		return __WINDOWS[indx].get();
	}

	void pollEvents() {
		for (auto& win : __WINDOWS) {
			win->pollEvents();
		}
	}

	// TODO: SUPPORT MORE RENDERERS
	void update() {
		if (RENDERER == nullptr) return;
		RENDERER->update();
	}

	// TODO: SUPPORT MORE RENDERERS
	void render() {
		if (RENDERER == nullptr) return;
		RENDERER->render();
	}

	void shutdown() {
		for (auto& win : __WINDOWS) {
			win->unblockPoll();
		}
	}
	// -------------------------

	// QUICK ACCESS ---
	uint32_t FRAME = 0;
	bool ENGINE_INITIALIZED = false;
	Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SHADER_FACTORY;
	rawrbox::Matrix4x4 TRANSFORM = {};
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
	// -----

	// OTHER INTERNAL
	bool __LIGHT_DIRTY__ = true;
	// --------------
} // namespace rawrbox::render
