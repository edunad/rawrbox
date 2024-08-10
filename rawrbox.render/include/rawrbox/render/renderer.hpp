#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/enums/draw.hpp>
#include <rawrbox/render/plugins/base.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/textures/render.hpp>
#include <rawrbox/render/textures/utils/blit.hpp>
#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/string.hpp>

#include <RefCntAutoPtr.hpp>
#include <ScopedQueryHelper.hpp>

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
	public:
		std::shared_ptr<rawrbox::TextureWEBP> texture = nullptr;

		float speed = 1.F;
		bool cover = false;
		rawrbox::Colorf background = rawrbox::Colors::Black();

		RawrboxIntro(float _speed = 1.F, bool _cover = true, const rawrbox::Colorf& _color = rawrbox::Colors::Black()) : speed(_speed), cover(_cover), background(_color) {}
		[[nodiscard]] bool isValid() const { return this->texture != nullptr; }
	};

	class RendererBase {
	protected:
		// PLUGINS -----
		std::map<std::string, std::unique_ptr<rawrbox::RenderPlugin>> _renderPlugins = {};
		// --------------

		// Post process renderer ----
		std::unique_ptr<rawrbox::TextureRender> _render = nullptr;
		// --------------------------

#ifdef _DEBUG
		// QUERIES ---
		std::unordered_map<std::string, std::unique_ptr<Diligent::ScopedQueryHelper>> _query = {};

		std::unordered_map<std::string, Diligent::QueryDataPipelineStatistics> _pipelineData = {};
		std::unordered_map<std::string, Diligent::QueryDataDuration> _durationData = {};
		// -------
#endif

		// INTRO ---
		std::atomic<bool> _introComplete = false;
		bool _skipIntros = false;

		rawrbox::RawrboxIntro* _currentIntro = nullptr;
		std::map<std::string, rawrbox::RawrboxIntro> _introList = {{"./assets/textures/rawrbox.webp", {1.4F, false}}}; // rawrbox intro, always the first
		//----

		std::function<void(const rawrbox::DrawPass& pass)> _drawCall = nullptr;
		std::function<void(const rawrbox::DrawPass& pass)> _tempRender = nullptr;

		rawrbox::Colorf _clearColor = rawrbox::Colors::Black();

		rawrbox::Vector2u _size = {};
		rawrbox::Vector2u _monitorSize = {};

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
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Renderer");
		std::unique_ptr<rawrbox::CameraBase> _camera = nullptr;
		std::unique_ptr<rawrbox::Stencil> _stencil = nullptr;
		std::unique_ptr<rawrbox::TextureBLIT> _GPUBlit = nullptr;
		// -------------

		// INTRO ------
		virtual void playIntro();
		virtual void introComplete();
		// ----------------

#ifdef _DEBUG
		// QUERIES ------
		virtual void beginQuery(const std::string& query);
		virtual void endQuery(const std::string& query);
		// ----------------
#endif
		virtual void clear();
		virtual void frame();

	public:
		std::function<void()> onIntroCompleted = nullptr;
		std::function<std::pair<uint32_t, uint32_t>()> overrideHEAP = nullptr;

		RendererBase(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2u& size, const rawrbox::Vector2u& monitorSize, const rawrbox::Colorf& clearColor = rawrbox::Colors::Black());
		RendererBase(const RendererBase&) = delete;
		RendererBase(RendererBase&&) = delete;
		RendererBase& operator=(const RendererBase&) = delete;
		RendererBase& operator=(RendererBase&&) = delete;
		virtual ~RendererBase();

		virtual void init(Diligent::DeviceFeatures features = {});
		virtual void resize(const rawrbox::Vector2u& size, const rawrbox::Vector2u& monitorSize);

		// PLUGINS ---------------------------
		template <typename T = rawrbox::RenderPlugin, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::RenderPlugin>)
		T* addPlugin(CallbackArgs&&... args) {
			if (this->_initialized) throw this->_logger->error("'addPlugin' must be called before 'init'!");
			auto renderPass = std::make_unique<T>(std::forward<CallbackArgs>(args)...);

			auto id = renderPass->getID();
			if (this->_renderPlugins.contains(id)) throw this->_logger->error("Plugin '{}' already registered!", id);

			auto* pass = renderPass.get();
			this->_renderPlugins[id] = std::move(renderPass);
			this->_logger->info("Registered renderer plugin '{}'", id);

			return pass;
		}

		template <typename T = rawrbox::RenderPlugin>
			requires(std::derived_from<T, rawrbox::RenderPlugin>)
		[[nodiscard]] T* getPlugin(const std::string& id) const {
			auto fnd = this->_renderPlugins.find(id);
			if (fnd == this->_renderPlugins.end()) return nullptr;

			return dynamic_cast<T*>(fnd->second.get());
		}

		[[nodiscard]] virtual bool hasPlugin(const std::string& plugin) const;
		[[nodiscard]] virtual const std::map<std::string, std::unique_ptr<rawrbox::RenderPlugin>>& getPlugins() const;
		// -----------------------------------

		virtual void setDrawCall(std::function<void(const rawrbox::DrawPass& pass)> call);

		virtual void update();
		virtual void render();

		// INTRO ------
		void skipIntros(bool skip); // :(
		void addIntro(const std::filesystem::path& webpPath, float speed = 1.F, bool cover = false, const rawrbox::Colorf& color = rawrbox::Colors::Black());
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
		[[nodiscard]] virtual rawrbox::Stencil* stencil() const;

		// WARNING: NOT THREAD SAFE!!
		[[nodiscard]] virtual Diligent::IDeviceContext* context() const;
		// WARNING: NOT THREAD SAFE!!
		[[nodiscard]] virtual Diligent::ISwapChain* swapChain() const;
		// WARNING: NOT THREAD SAFE!!
		[[nodiscard]] virtual Diligent::IRenderDevice* device() const;

		[[nodiscard]] virtual Diligent::ITextureView* getDepth() const;
		[[nodiscard]] virtual Diligent::ITextureView* getColor(bool rt = false) const;

		[[nodiscard]] virtual std::filesystem::path getShadersDirectory() const;
		[[nodiscard]] virtual const Diligent::RENDER_DEVICE_TYPE& getRenderType() const;

#ifdef _DEBUG
		[[nodiscard]] virtual const Diligent::QueryDataPipelineStatistics& getPipelineStats(const std::string& query);
		[[nodiscard]] virtual const Diligent::QueryDataDuration& getDurationStats(const std::string& query);
#endif

		[[nodiscard]] virtual const rawrbox::Vector2u& getSize() const;

		[[nodiscard]] virtual bool getVSync() const;
		virtual void setVSync(bool vsync);

		virtual void gpuPick(const rawrbox::Vector2i& pos, const std::function<void(uint32_t)>& callback);
		//  ------
	};
} // namespace rawrbox
