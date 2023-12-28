#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/enums/draw.hpp>
#include <rawrbox/render/plugins/base.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/textures/render.hpp>

#include <RefCntAutoPtr.hpp>

#include <DeviceContext.h>
#include <RenderDevice.h>
#include <SwapChain.h>
#include <fmt/printf.h>

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
		// PLUGINS -----
		std::map<std::string, std::unique_ptr<rawrbox::RenderPlugin>> _renderPlugins = {};
		// --------------

		std::unique_ptr<rawrbox::TextureRender> _render = nullptr;
		std::unique_ptr<rawrbox::TextureRender> _decals = nullptr;

		// INTRO ---
		bool _introComplete = false;
		bool _skipIntros = false;

		rawrbox::RawrboxIntro* _currentIntro = nullptr;
		std::map<std::string, rawrbox::RawrboxIntro> _introList = {{"./assets/textures/rawrbox.webp", {nullptr, 1.4F, false}}}; // rawrbox intro, always the first
		//----

		std::function<void(const rawrbox::DrawPass& pass)> _drawCall = nullptr;
		std::function<void(const rawrbox::DrawPass& pass)> _tempRender = nullptr;

		rawrbox::Colorf _clearColor = rawrbox::Colors::Black();

		rawrbox::Vector2i _size = {};
		rawrbox::Vector2i _monitorSize = {};

		Diligent::NativeWindow _window = {};

		bool _vsync = false;
		bool _initialized = false;

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
		virtual void introComplete();
		// ----------------

		virtual void clear();
		virtual void frame();
		//  virtual void finalRender();
		//   virtual void gpuCheck();

	public:
		std::function<void()> onIntroCompleted = nullptr;

		RendererBase(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2i& size, const rawrbox::Vector2i& monitorSize, const rawrbox::Colorf& clearColor = rawrbox::Colors::Black());
		RendererBase(const RendererBase&) = delete;
		RendererBase(RendererBase&&) = delete;
		RendererBase& operator=(const RendererBase&) = delete;
		RendererBase& operator=(RendererBase&&) = delete;
		virtual ~RendererBase();

		virtual void init(Diligent::DeviceFeatures features = {});
		virtual void resize(const rawrbox::Vector2i& size, const rawrbox::Vector2i& monitorSize);

		// PLUGINS ---------------------------
		template <typename T = rawrbox::RenderPlugin, typename... CallbackArgs>
		T* addPlugin(CallbackArgs&&... args) {
			if (this->_initialized) throw std::runtime_error("[RawrBox-Renderer] 'addPlugin' must be called before 'init'!");
			auto renderPass = std::make_unique<T>(std::forward<CallbackArgs>(args)...);
			auto p = renderPass.get();

			this->_renderPlugins[p->getID()] = std::move(renderPass);
			fmt::print("[RawrBox-Renderer] Registered new plugin '{}'\n", p->getID());

			return p;
		}

		template <typename T = rawrbox::RenderPlugin>
		[[nodiscard]] T* getPlugin(const std::string& id) const {
			auto fnd = this->_renderPlugins.find(id);
			if (fnd == this->_renderPlugins.end()) return nullptr;
			return dynamic_cast<T*>(fnd->second.get());
		}
		// -----------------------------------

		virtual void setDrawCall(std::function<void(const rawrbox::DrawPass& pass)> call);

		virtual void update();
		virtual void render();

		// INTRO ------
		void skipIntros(bool skip); // :(
		void addIntro(const std::filesystem::path& webpPath, float speed = 1.F, bool cover = false);
		// ----------------

		// CAMERA ------
		template <class T = rawrbox::CameraBase, typename... CallbackArgs>
		T* setupCamera(CallbackArgs&&... args) {
			this->_camera = std::make_unique<T>(std::forward<CallbackArgs>(args)...);
			this->setMainCamera(this->_camera.get());

			return dynamic_cast<T*>(this->_camera.get());
		}

		virtual void setMainCamera(rawrbox::CameraBase* camera) const;
		[[nodiscard]] virtual rawrbox::CameraBase* camera() const;
		// ----------------

		// Utils ----
		[[nodiscard]] virtual rawrbox::Stencil& stencil() const;

		[[nodiscard]] virtual Diligent::IDeviceContext* context() const;
		[[nodiscard]] virtual Diligent::ISwapChain* swapChain() const;
		[[nodiscard]] virtual Diligent::IRenderDevice* device() const;

		[[nodiscard]] virtual Diligent::ITextureView* getDepth() const;
		[[nodiscard]] virtual Diligent::ITextureView* getColor(bool rt = false) const;

		//[[nodiscard]] virtual const bgfx::TextureHandle getMask() const;
		//[[nodiscard]] virtual const bgfx::TextureHandle getGPUPick() const;

		[[nodiscard]] virtual const rawrbox::Vector2i& getSize() const;
		[[nodiscard]] virtual bool getVSync() const;
		virtual void setVSync(bool vsync);

		// virtual void gpuPick(const rawrbox::Vector2i& pos, std::function<void(uint32_t)> callback);
		//  ------
	};
} // namespace rawrbox
