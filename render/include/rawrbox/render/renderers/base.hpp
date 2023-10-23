#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <Common/interface/RefCntAutoPtr.hpp>

#include <Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <Graphics/GraphicsEngine/interface/SwapChain.h>

#include <filesystem>
#include <functional>
#include <map>

namespace rawrbox {
	class TextureWEBP;
	struct RawrboxIntro {
		std::shared_ptr<rawrbox::TextureWEBP> texture;
		float speed;
		bool cover;
	};

	class RendererBase {
	protected:
		std::unique_ptr<rawrbox::TextureRender> _render = nullptr;
		std::unique_ptr<rawrbox::TextureRender> _decals = nullptr;

		// INTRO ---
		std::function<void()> _tempWorldRender = nullptr;
		std::function<void()> _tempOverlayRender = nullptr;

		bool _introComplete = false;
		bool _skipIntros = false;

		rawrbox::RawrboxIntro* _currentIntro = nullptr;
		std::map<std::string, rawrbox::RawrboxIntro> _introList = {{"./assets/textures/rawrbox.webp", {nullptr, 1.4F, false}}}; // rawrbox intro, always the first
		//----

		rawrbox::Colorf _clearColor = rawrbox::Colors::Black();
		rawrbox::Vector2i _size = {};
		Diligent::NativeWindow _window = {};

		bool _vsync = false;
		Diligent::RENDER_DEVICE_TYPE _type = Diligent::RENDER_DEVICE_TYPE_UNDEFINED;

		// HANDLES ---
		Diligent::RefCntAutoPtr<Diligent::IDeviceContext> _context;
		Diligent::RefCntAutoPtr<Diligent::ISwapChain> _swapChain;
		Diligent::RefCntAutoPtr<Diligent::IRenderDevice> _device;
		Diligent::RefCntAutoPtr<Diligent::IEngineFactory> _engineFactory;
		// ----

		// OTHER HANDLES
		std::unique_ptr<rawrbox::CameraBase> _camera = nullptr;
		std::unique_ptr<rawrbox::Stencil> _stencil = nullptr;
		// -------------

		// GPU PICKING ----
		// uint32_t _gpuReadFrame = 0;
		// std::array<uint8_t, rawrbox::GPU_PICK_SAMPLE_SIZE> _gpuPixelData = {};
		// std::vector<std::function<void(uint32_t)>> _gpuPickCallbacks = {};
		// ----------------

		// INTRO ------
		virtual void playIntro();
		virtual void completeIntro();
		// ----------------

		virtual void clear();
		virtual void frame();
		virtual void finalRender();
		// virtual void gpuCheck();

	public:
		std::function<void()> worldRender = nullptr;
		std::function<void()> overlayRender = nullptr;
		std::function<void()> postRender = nullptr;

		std::function<void()> onIntroCompleted = nullptr;

		RendererBase(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2i& size, const rawrbox::Colorf& clearColor = rawrbox::Colors::Black());
		RendererBase(const RendererBase&) = delete;
		RendererBase(RendererBase&&) = delete;
		RendererBase& operator=(const RendererBase&) = delete;
		RendererBase& operator=(RendererBase&&) = delete;
		virtual ~RendererBase();

		virtual void init();
		virtual void resize(const rawrbox::Vector2i& size);

		virtual void setWorldRender(std::function<void()> render);
		virtual void setOverlayRender(std::function<void()> render);
		virtual void overridePostWorld(std::function<void()> post);

		virtual void update();
		virtual void render();

		// INTRO ------
		void skipIntros(bool skip); // :(
		void addIntro(const std::filesystem::path& webpPath, float speed = 1.F, bool cover = false);
		// ----------------

		// virtual void finalRender();
		// virtual void bindRenderUniforms();

		template <class T = rawrbox::CameraBase, typename... CallbackArgs>
		T* setupCamera(CallbackArgs&&... args) {
			this->_camera = std::make_unique<T>(std::forward<CallbackArgs>(args)...);
			return dynamic_cast<T*>(this->_camera.get());
		}

		// Utils ----
		[[nodiscard]] virtual rawrbox::CameraBase* camera() const;
		[[nodiscard]] virtual rawrbox::Stencil* stencil() const;

		[[nodiscard]] virtual Diligent::IDeviceContext* context() const;
		[[nodiscard]] virtual Diligent::ISwapChain* swapChain() const;
		[[nodiscard]] virtual Diligent::IRenderDevice* device() const;

		[[nodiscard]] virtual Diligent::ITextureView* getDepth() const;
		[[nodiscard]] virtual Diligent::ITextureView* getColor(bool rt = false) const;
		//[[nodiscard]] virtual const bgfx::TextureHandle getMask() const;
		//[[nodiscard]] virtual const bgfx::TextureHandle getGPUPick() const;

		[[nodiscard]] virtual const rawrbox::Vector2i getSize() const;
		[[nodiscard]] virtual bool getVSync() const;
		virtual void setVSync(bool vsync);
		// virtual void gpuPick(const rawrbox::Vector2i& pos, std::function<void(uint32_t)> callback);
		//  ------

		[[nodiscard]] virtual bool supported() const;
	};
} // namespace rawrbox
