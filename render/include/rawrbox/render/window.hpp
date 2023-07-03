#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/camera/perspective.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/utils/event.hpp>

#include <bgfx/platform.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

struct GLFWwindow;
namespace rawrbox {
	// ------FLAGS

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
			const uint32_t VSYNC = 1 << 6;
			const uint32_t MSAA = 1 << 7;
			const uint32_t MULTI_THREADED = 1 << 8;
		}; // namespace Features

		namespace Debug {
			const uint32_t WIREFRAME = 1 << 9;
			const uint32_t STATS = 1 << 10;
			const uint32_t TEXT = 1 << 11;
			const uint32_t PROFILER = 1 << 12;

		}; // namespace Debug
	};         // namespace WindowFlags

	// NOLINTEND{unused-const-variable}
	//  --------------------

	class Window;

	// ------EVENTS
	using OnFocusCallback = Event<Window&, bool>;
	using OnCharCallback = Event<Window&, uint32_t>;
	using OnResizeCallback = Event<Window&, const Vector2i&>;
	using OnScrollCallback = Event<Window&, const Vector2i&, const Vector2i&>;
	using OnMouseMoveCallback = Event<Window&, const Vector2i&>;
	using OnKeyCallback = Event<Window&, uint32_t, uint32_t, uint32_t, uint32_t>;
	using OnMouseKeyCallback = Event<Window&, const Vector2i&, uint32_t, uint32_t, uint32_t>;
	using OnWindowClose = Event<Window&>;
	// --------------------

	class Window {
	private:
		void* _handle = nullptr;
		void* _cursor = nullptr;

		uint32_t _windowFlags = 0;
		uint32_t _resetFlags = BGFX_RESET_NONE;
		uint32_t _debugFlags = BGFX_DEBUG_NONE;

		// Stencil ---
		std::unique_ptr<rawrbox::Stencil> _stencil = nullptr;
		std::unique_ptr<rawrbox::CameraBase> _camera = nullptr;
		// -------

		// Default settings
		std::string _title = "RawrBOX - Window";
		bgfx::RendererType::Enum _renderType = bgfx::RendererType::Count;
		int _monitor = -1;

		rawrbox::Vector2i _size = {};
		// -----

		// Screen Utils --
		std::unordered_map<int, rawrbox::Vector2i> _screenSizes = {};
		// --------

		// ------CALLBACKS
		static void callbacks_focus(GLFWwindow* whandle, int focus);
		static void callbacks_char(GLFWwindow* whandle, unsigned int ch);
		static void callbacks_scroll(GLFWwindow* whandle, double x, double y);
		static void callbacks_mouseMove(GLFWwindow* whandle, double x, double y);
		static void callbacks_resize(GLFWwindow* whandle, int width, int height);
		static void callbacks_mouseKey(GLFWwindow* whandle, int button, int action, int mods);
		static void callbacks_key(GLFWwindow* whandle, int key, int scancode, int action, int mods);
		static void callbacks_windowClose(GLFWwindow* whandle);
		// --------------------

	public:
		bool hasFocus = true;

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
		void initializeBGFX(uint32_t clearColor = 0x000000FF);

		void setMonitor(int monitor);
		void setRenderer(bgfx::RendererType::Enum render);
		void setTitle(const std::string& title);

		template <typename T = rawrbox::CameraPerspective, typename... CallbackArgs>
		T* setupCamera(CallbackArgs&&... args) {
			this->_camera = std::make_unique<T>(std::forward<CallbackArgs>(args)...);
			rawrbox::MAIN_CAMERA = this->_camera.get();

			return dynamic_cast<T*>(this->_camera.get());
		}

		// CURSOR ------
		void hideCursor(bool hidden);
		void setCursor(uint32_t cursor);
		void setCursor(const std::array<uint8_t, 1024>& cursor); // Max size 16x16 (4 pixel channel)
		// --------------------

		void shutdown();
		void update();

		// UPDATE ------
		void pollEvents();
		void unblockPoll();
		// --------------------
		// DRAW -----
		void clear();
		void upload();
		void frame() const;
		// -----------

		// UTILS ---------------
		void close();
		[[nodiscard]] virtual bool getShouldClose() const;
		virtual void setShouldClose(bool close) const;

		[[nodiscard]] virtual rawrbox::Vector2i getSize() const;
		[[nodiscard]] virtual float getAspectRatio() const;

		[[nodiscard]] virtual rawrbox::Vector2i getMousePos() const;

		[[nodiscard]] virtual uint32_t getWindowFlags() const;

		[[nodiscard]] virtual rawrbox::Stencil& getStencil() const;

		[[nodiscard]] virtual bool isKeyDown(int key) const;
		[[nodiscard]] virtual bool isMouseDown(int key) const;

		[[nodiscard]] virtual bool isRendererSupported(bgfx::RendererType::Enum render) const;
		[[nodiscard]] virtual const std::unordered_map<int, rawrbox::Vector2i>& getScreenSizes() const;
		// --------------------

		virtual ~Window();
		Window();
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
	};
} // namespace rawrbox
