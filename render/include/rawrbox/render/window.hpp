#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/utils/event.hpp>

#include <Graphics/GraphicsEngine/interface/GraphicsTypes.h>
#include <fmt/format.h>

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
		}; // namespace Features
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

	struct WindowSettings {
		rawrbox::Vector2i size = {};
		uint32_t flags = 0;
		int monitor = -1;
		std::string title = "RawrBox";
	};

	class Window {
	private:
		GLFWwindow* _handle = nullptr;

		// CURSOR ------
		void* _cursor = nullptr;
		std::array<uint8_t, 16 * 16 * 4> _cursorPixels = {};
		// -----------

		Diligent::RENDER_DEVICE_TYPE _renderType = Diligent::RENDER_DEVICE_TYPE::RENDER_DEVICE_TYPE_COUNT;
		rawrbox::WindowSettings _settings = {};
		bool _hasFocus = false;

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

	public:
		virtual ~Window();
		Window(Diligent::RENDER_DEVICE_TYPE renderType = Diligent::RENDER_DEVICE_TYPE_UNDEFINED);
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

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

		void init(int width, int height, uint32_t flags = WindowFlags::NONE);

		void setMonitor(int monitor);
		void setTitle(const std::string& title);

		// CURSOR ------
		void hideCursor(bool hidden);
		void setCursor(uint32_t cursor);
		void setCursor(const std::array<uint8_t, 1024>& cursor); // Max size 16x16 (4 pixel channel)
		// --------------------

		void shutdown();

		// UPDATE ------
		void pollEvents();
		void unblockPoll();
		// --------------------

		// UTILS ---------------
		void close();

		[[nodiscard]] virtual rawrbox::Vector2i getSize() const;
		[[nodiscard]] virtual float getAspectRatio() const;
		[[nodiscard]] virtual rawrbox::Vector2i getMousePos() const;
		[[nodiscard]] virtual uint32_t getWindowFlags() const;
		[[nodiscard]] virtual Diligent::NativeWindow getHandle() const;

		[[nodiscard]] virtual bool isKeyDown(int key) const;
		[[nodiscard]] virtual bool isMouseDown(int key) const;

		[[nodiscard]] virtual const std::unordered_map<int, rawrbox::Vector2i>& getScreenSizes() const;

		[[nodiscard]] virtual bool hasFocus() const;
		// --------------------
	};
} // namespace rawrbox
