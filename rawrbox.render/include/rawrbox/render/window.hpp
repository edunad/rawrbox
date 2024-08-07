#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/utils/event.hpp>

#include <GraphicsTypes.h>
#include <fmt/format.h>

struct GLFWwindow;
struct GLFWmonitor;
struct GLFWcursor;

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
		}; // namespace Features
	};         // namespace WindowFlags

	// NOLINTEND{unused-const-variable}
	//  --------------------

	class Window;
	enum class ENGINE_THREADS;

	// ------EVENTS
	using OnFocusCallback = rawrbox::Event<Window&, bool>;
	using OnCharCallback = rawrbox::Event<Window&, uint32_t>;
	using OnResizeCallback = rawrbox::Event<Window&, const Vector2u&, const Vector2u&>;
	using OnScrollCallback = rawrbox::Event<Window&, const Vector2i&, const Vector2i&>;
	using OnMouseMoveCallback = rawrbox::Event<Window&, const Vector2i&>;
	using OnWindowMoveCallback = rawrbox::Event<Window&, const Vector2i&>;
	using OnKeyCallback = rawrbox::Event<Window&, uint32_t, uint32_t, uint32_t, uint32_t>;
	using OnMouseKeyCallback = rawrbox::Event<Window&, const Vector2i&, uint32_t, uint32_t, uint32_t>;
	using OnWindowClose = rawrbox::Event<Window&>;
	// --------------------

	struct WindowSettings {
		rawrbox::Vector2u size = {};
		rawrbox::Vector2i pos = {-1, -1};

		uint32_t flags = 0;
		int monitor = -1;
		std::string title = "RawrBox";
	};

	class Window {

	private:
		GLFWwindow* _handle = nullptr;
		std::unique_ptr<rawrbox::RendererBase> _renderer = nullptr;

		// CURSOR ------
		GLFWcursor* _cursor = nullptr;
		std::array<uint8_t, 16 * 16 * 4> _cursorPixels = {};
		rawrbox::Vector2i _mousePos = {};
		// -----------

		Diligent::RENDER_DEVICE_TYPE _renderType = Diligent::RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_UNDEFINED;
		rawrbox::WindowSettings _settings = {};
		bool _hasFocus = false;

		// Screen Utils --
		std::unordered_map<int, rawrbox::Vector2u> _screenSizes = {};
		// --------

		// STATIC ----
		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static std::vector<std::unique_ptr<rawrbox::Window>> __WINDOWS;
		static Diligent::RENDER_DEVICE_TYPE __RENDER_TYPE;
		// -------------

		// CALLBACKS ------
		static void callbacks_focus(GLFWwindow* whandle, int focus);
		static void callbacks_char(GLFWwindow* whandle, unsigned int ch);
		static void callbacks_scroll(GLFWwindow* whandle, double x, double y);
		static void callbacks_mouseMove(GLFWwindow* whandle, double x, double y);
		static void callbacks_pos(GLFWwindow* whandle, int x, int y);
		static void callbacks_resize(GLFWwindow* whandle, int width, int height);
		static void callbacks_mouseKey(GLFWwindow* whandle, int button, int action, int mods);
		static void callbacks_key(GLFWwindow* whandle, int key, int scancode, int action, int mods);
		static void callbacks_windowClose(GLFWwindow* whandle);
		// --------------------

		Window(Diligent::RENDER_DEVICE_TYPE renderType = Diligent::RENDER_DEVICE_TYPE_UNDEFINED);
		static void setActiveRenderer(rawrbox::RendererBase* r);

	public:
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		virtual ~Window();

		// STATIC ---
		static rawrbox::Window* createWindow(Diligent::RENDER_DEVICE_TYPE render = Diligent::RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_UNDEFINED);
		static rawrbox::Window* getWindow(size_t indx = 0);

		static void pollEvents();
		static void update();
		static void render();
		static void shutdown(rawrbox::ENGINE_THREADS thread);
		// --------

		// Renderer ------
		template <class T = rawrbox::RendererBase, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::RendererBase>)
		rawrbox::RendererBase* createRenderer(CallbackArgs&&... args) {
			_renderer = std::make_unique<T>(rawrbox::Window::__RENDER_TYPE, this->getHandle(), this->getSize(), this->getMonitorSize(), std::forward<CallbackArgs>(args)...);

			// Setup resize ----
			this->onResize += [this](auto&, auto& size, auto& monitorSize) {
				_renderer->resize(size, monitorSize);
			};
			// -----------------

			setActiveRenderer(_renderer.get());
			return _renderer.get();
		};
		// ---------------

		// ------CALLBACKS
		OnKeyCallback onKey;
		OnCharCallback onChar;
		OnFocusCallback onFocus;
		OnResizeCallback onResize;
		OnScrollCallback onMouseScroll;
		OnMouseKeyCallback onMouseKey;
		OnMouseMoveCallback onMouseMove;
		OnWindowMoveCallback onWindowMove;
		OnWindowClose onWindowClose;
		// --------------------

		void init(uint32_t width, uint32_t height, uint32_t flags = WindowFlags::NONE);

		void setMonitor(int monitor);
		void setTitle(const std::string& title);
		void setOpacity(float opacity);

#ifdef _WIN32
		void alert(); // Blink application, only available in windows
#endif

		// CURSOR ------
		void hideCursor(bool hidden);
		void setCursor(uint32_t cursor);
		void setCursor(const std::array<uint8_t, 1024>& cursor); // Max size 16x16 (4 pixel channel)
		// --------------------

		// UTILS ---------------
		void close();

		[[nodiscard]] virtual const rawrbox::Vector2u& getSize() const;

		virtual void setPos(const rawrbox::Vector2i& pos);
		[[nodiscard]] virtual const rawrbox::Vector2i& getPos() const;

		[[nodiscard]] virtual rawrbox::Vector2u getMonitorSize() const;

		[[nodiscard]] virtual float getAspectRatio() const;
		[[nodiscard]] virtual const rawrbox::Vector2i& getMousePos() const;
		[[nodiscard]] virtual uint32_t getWindowFlags() const;

		[[nodiscard]] virtual GLFWwindow* getGLFWHandle() const;
		[[nodiscard]] virtual Diligent::NativeWindow getHandle() const;
		[[nodiscard]] virtual rawrbox::RendererBase& getRenderer() const;

		[[nodiscard]] virtual bool isKeyDown(int key) const;
		[[nodiscard]] virtual bool isMouseDown(int key) const;

		[[nodiscard]] virtual const std::unordered_map<int, rawrbox::Vector2u>& getScreenSizes() const;

		[[nodiscard]] virtual bool hasFocus() const;

		[[nodiscard]] virtual GLFWmonitor* getWindowMonitor() const;
		// --------------------
	};
} // namespace rawrbox
