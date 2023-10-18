#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/utils/event.hpp>

#include <Graphics/GraphicsEngine/interface/EngineFactory.h>
#include <Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <Graphics/GraphicsEngine/interface/SwapChain.h>
#include <fmt/format.h>

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

struct GLFWwindow;
namespace rawrbox {
	// FLAGS ------
	// NOLINTBEGIN{unused-const-variable}
	namespace WindowFlags {
		const uint32_t NONE = 0;

		namespace Window {
			const uint32_t FULLSCREEN = 1 << 1;
			const uint32_t WINDOWED = 1 << 2;
			const uint32_t BORDERLESS = 1 << 3;
		}; // namespace Window

		namespace Features {
			const uint32_t TRANSPARENT_BUFFER = 1 << 4;
			const uint32_t RESIZABLE = 1 << 5;
			// const uint32_t VSYNC = 1 << 6;
			// const uint32_t MSAA = 1 << 7;
			// const uint32_t MULTI_THREADED = 1 << 8;
		}; // namespace Features

		namespace Debug {
			const uint32_t WIREFRAME = 1 << 9;
			// const uint32_t STATS = 1 << 10;
			// const uint32_t TEXT = 1 << 11;
			// const uint32_t PROFILER = 1 << 12;
		}; // namespace Debug
	};         // namespace WindowFlags

	// NOLINTEND{unused-const-variable}
	//  --------------------

	class Window;

	// ------EVENTS
	using OnFocusCallback = rawrbox::Event<Window&, bool>;
	using OnCharCallback = rawrbox::Event<Window&, uint32_t>;
	using OnResizeCallback = rawrbox::Event<Window&, const Vector2i&>;
	using OnScrollCallback = rawrbox::Event<Window&, const Vector2i&, const Vector2i&>;
	using OnMouseMoveCallback = rawrbox::Event<Window&, const Vector2i&>;
	using OnKeyCallback = rawrbox::Event<Window&, uint32_t, uint32_t, uint32_t, uint32_t>;
	using OnMouseKeyCallback = rawrbox::Event<Window&, const Vector2i&, uint32_t, uint32_t, uint32_t>;
	using OnWindowClose = rawrbox::Event<Window&>;
	// --------------------

	class TextureWEBP;
	struct RawrboxIntro {
	public:
		std::shared_ptr<rawrbox::TextureWEBP> texture;
		float speed;
		bool cover;
	};

	class Window {
	private:
		std::function<void()> _overlay = nullptr;
		std::function<void()> _world = nullptr;

		GLFWwindow* _handle = nullptr;

		// HANDLES -----
		Diligent::RefCntAutoPtr<Diligent::IEngineFactory> _pEngineFactory;
		// -------------

		// CURSOR ------
		void* _cursor = nullptr;
		std::array<uint8_t, 16 * 16 * 4> _cursorPixels = {};
		// -----------

		uint32_t _windowFlags = 0;

		bool _hasFocus = true;

		// Intro -----
		bool _introComplete = false;
		bool _skipIntros = false;
		rawrbox::RawrboxIntro* _currentIntro = nullptr;
		std::map<std::string, rawrbox::RawrboxIntro> _introList = {{"./assets/textures/rawrbox.webp", {nullptr, 1.4F, false}}}; // rawrbox intro, always the first
		//  ---

		// Data ---
		std::unique_ptr<rawrbox::Stencil> _stencil = nullptr;
		std::unique_ptr<rawrbox::CameraBase> _camera = nullptr;
		std::unique_ptr<rawrbox::RendererBase> _renderer = nullptr;
		// -------

		// Default settings
		std::string _title = "RawrBox - Window";
		Diligent::RENDER_DEVICE_TYPE _renderType = Diligent::RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_COUNT;
		int _monitor = -1;

		rawrbox::Vector2i _size = {};
		// -----

		// Screen Utils --
		std::unordered_map<int, rawrbox::Vector2i> _screenSizes = {};
		// --------

		// CALLBACKS ------
		static void callbacks_focus(GLFWwindow* whandle, int focus);
		static void callbacks_char(GLFWwindow* whandle, unsigned int ch);
		static void callbacks_scroll(GLFWwindow* whandle, double x, double y);
		static void callbacks_mouseMove(GLFWwindow* whandle, double x, double y);
		static void callbacks_resize(GLFWwindow* whandle, int width, int height);
		static void callbacks_mouseKey(GLFWwindow* whandle, int button, int action, int mods);
		static void callbacks_key(GLFWwindow* whandle, int key, int scancode, int action, int mods);
		static void callbacks_windowClose(GLFWwindow* whandle);
		// --------------------

		// INTRO ------
		void playIntro();
		// --------------------

		// ENGINE -------
		Diligent::RENDER_DEVICE_TYPE getRenderDeviceType();
		// --------------

	public:
		rawrbox::Event<> onIntroCompleted;

		// ------CALLBACKS
		OnKeyCallback onKey;
		OnCharCallback onChar;
		OnFocusCallback onFocus;
		OnResizeCallback onResize;
		OnScrollCallback onMouseScroll;
		OnMouseKeyCallback onMouseKey;
		OnMouseMoveCallback onMouseMove;
		OnWindowClose onWindowClose;
		// --------------------

		void create(int width, int height, uint32_t flags = WindowFlags::NONE);
		void initializeEngine();

		void setMonitor(int monitor);
		void setTitle(const std::string& title);

		template <class T = rawrbox::CameraBase, typename... CallbackArgs>
		T* setupCamera(CallbackArgs&&... args) {
			this->_camera = std::make_unique<T>(std::forward<CallbackArgs>(args)...);
			rawrbox::MAIN_CAMERA = this->_camera.get();

			return dynamic_cast<T*>(this->_camera.get());
		}

		// RENDERER -----
		template <class T = RendererBase, typename... CallbackArgs>
		void setRenderer(const rawrbox::Colorf& clearColor, Diligent::RENDER_DEVICE_TYPE render, std::function<void()> overlay, std::function<void()> world, CallbackArgs&&... args) {
			this->_renderType = render;
			this->_overlay = overlay;
			this->_world = world;

			if (render == Diligent::RENDER_DEVICE_TYPE_GL || render == Diligent::RENDER_DEVICE_TYPE_GLES) {
				rawrbox::MTX_RIGHT_HANDED = true;
			}

			this->_renderer = std::make_unique<T>(clearColor, std::forward<CallbackArgs>(args)...);
			rawrbox::RENDERER = this->_renderer.get();
		}

		// ----------

		void overridePostWorld(std::function<void()> fnc);

		// CURSOR ------
		void hideCursor(bool hidden);
		void setCursor(uint32_t cursor);
		void setCursor(const std::array<uint8_t, 1024>& cursor); // Max size 16x16 (4 pixel channel)
		// --------------------

		void shutdown();
		void update();

		// INTRO ------
		void skipIntros(bool skip); // :(
		void addIntro(const std::filesystem::path& webpPath, float speed = 1.F, bool cover = false);
		// ----------------

		// UPDATE ------
		void pollEvents();
		void unblockPoll();
		// --------------------

		// DRAW -----
		void render() const;
		// -----------

		// UTILS ---------------
		void close();

		[[nodiscard]] virtual rawrbox::Vector2i getSize() const;
		[[nodiscard]] virtual float getAspectRatio() const;

		[[nodiscard]] virtual rawrbox::Vector2i getMousePos() const;

		[[nodiscard]] virtual uint32_t getWindowFlags() const;

		[[nodiscard]] virtual rawrbox::Stencil& getStencil() const;

		[[nodiscard]] virtual bool isKeyDown(int key) const;
		[[nodiscard]] virtual bool isMouseDown(int key) const;

		//[[nodiscard]] virtual bool isRendererSupported(bgfx::RendererType::Enum render) const;
		[[nodiscard]] virtual const std::unordered_map<int, rawrbox::Vector2i>& getScreenSizes() const;

		[[nodiscard]] virtual bool hasFocus() const;
		// --------------------

		virtual ~Window();
		Window();
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
	};
} // namespace rawrbox
