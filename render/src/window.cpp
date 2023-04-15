#include <rawrbox/render/window.h>

#include <bgfx/bgfx.h>
#include <bx/bx.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
#error "GLFW 3.2 or later is required"
#endif // GLFW_VERSION_MINOR < 2

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if RAWRBOX_USE_WAYLAND
#include <wayland-egl.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#else
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif //
#include <GLFW/glfw3native.h>

#include <map>
#include <stdexcept>
#include <fmt/printf.h>

#define GLFWHANDLE reinterpret_cast<GLFWwindow*>(_handle)

namespace rawrBox {
	static Window& glfwHandleToRenderer(GLFWwindow* ptr) {
		return *static_cast<Window*>(glfwGetWindowUserPointer(ptr));
	}

	static void glfw_errorCallback(int error, const char* description) {
		fmt::print("GLFW error {}: {}\n", error, description);
	}

	static void* glfwNativeWindowHandle(GLFWwindow* _window) {
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
		wl_egl_window* win_impl = static_cast<wl_egl_window*>(glfwGetWindowUserPointer(_window));
		if (!win_impl) {
			int width, height;
			glfwGetWindowSize(_window, &width, &height);
			struct wl_surface* surface = (struct wl_surface*)glfwGetWaylandWindow(_window);
			if (!surface)
				return nullptr;
			win_impl = wl_egl_window_create(surface, width, height);
			glfwSetWindowUserPointer(_window, (void*)(uintptr_t)win_impl);
		}

		return (void*)(uintptr_t)win_impl;
#else
		return (void*)(uintptr_t)glfwGetX11Window(_window);
#endif
#elif BX_PLATFORM_OSX
		return glfwGetCocoaWindow(_window);
#elif BX_PLATFORM_WINDOWS
		return glfwGetWin32Window(_window);
#endif // BX_PLATFORM_

		throw std::runtime_error("[RawrBox-Render] Failed to detect platform");
	}

	static void* getNativeDisplayHandle() {
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
		return glfwGetWaylandDisplay();
#else
		return glfwGetX11Display();
#endif
#else
		return nullptr;
#endif // BX_PLATFORM_*
	}

	void Window::initialize(int width, int height, uint32_t flags) {

		glfwSetErrorCallback(glfw_errorCallback);
		if (!glfwInit()) throw std::runtime_error("[RawrBox-Window] Failed to initialize glfw");
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable opengl
		// -------------

		std::map<int, int> properties = {};

		// Monitor selection ----
		GLFWmonitor* mon = glfwGetPrimaryMonitor();
		if (this->_monitor >= 0) {
			int monitorscount = 0;
			auto monitors = glfwGetMonitors(&monitorscount);
			if (this->_monitor < monitorscount) {
				mon = monitors[this->_monitor];
				properties[GLFW_VISIBLE] = 0;
			}
		}

		if (mon == nullptr) throw std::runtime_error("[RawrBox-Window] Failed to get primary window");
		// ----------------------------

		// Fullscreen / borderless
		const GLFWvidmode* mode = glfwGetVideoMode(mon);
		bool windowed = (flags & WindowFlags::Window::WINDOWED) > 0;
		bool borderless = (flags & WindowFlags::Window::BORDERLESS) > 0;
		bool fullscreen = (flags & WindowFlags::Window::FULLSCREEN) > 0;

		if (windowed && (borderless || fullscreen)) throw std::runtime_error("[RawrBox-Window] Only one window attribute can be selected");
		if (borderless && (windowed || fullscreen)) throw std::runtime_error("[RawrBox-Window] Only one window attribute can be selected");
		if (fullscreen && (windowed || borderless)) throw std::runtime_error("[RawrBox-Window] Only one window attribute can be selected");

		if (fullscreen) {
			width = mode->width;
			height = mode->height;
		} else if ((width >= mode->width || height >= mode->height)) {
			width = mode->width;
			height = mode->height;
		}

		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		// -----------

		// Window properties
		properties[GLFW_RESIZABLE] = (flags & WindowFlags::Features::RESIZABLE) > 0 ? 1 : 0;
		properties[GLFW_DECORATED] = borderless || fullscreen ? 0 : 1;
		for (const auto& pair : properties) {
			glfwWindowHint(pair.first, pair.second);
		}
		// ------

		auto glfwHandle = glfwCreateWindow(width, height, this->_title.c_str(), windowed ? nullptr : mon, nullptr);
		if (glfwHandle == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Window] Failed to initialize window [{} - {}x{}]", this->_title, width, height));

		this->_handle = glfwHandle;
		glfwSetWindowUserPointer(glfwHandle, this);

		// Center window
		if (windowed) {
			int monx = 0, mony = 0, monw = 0, monh = 0;
			glfwGetMonitorPos(mon, &monx, &mony);

			glfwSetWindowPos(glfwHandle, monx + mode->width / 2 - width / 2, mony + mode->height / 2 - height / 2);
			glfwShowWindow(glfwHandle);
		}
// ------

// Set icon
#ifdef WIN32
		HANDLE hIcon = LoadIconW(GetModuleHandleW(nullptr), L"GLFW_ICON");
		if (!hIcon) {
			// No user-provided icon found, load default icon
			hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		}

		HWND hwnd = glfwGetWin32Window(glfwHandle);
		::SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		::SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
#endif
		// ---------------

		if ((flags & WindowFlags::Features::MULTI_THREADED) == 0) bgfx::renderFrame(); // Disable multi-threading

		bgfx::Init init;
		init.type = this->_renderType;
		init.resolution.width = static_cast<uint32_t>(width);
		init.resolution.height = static_cast<uint32_t>(height);

		auto resetFlags = BGFX_RESET_NONE;
		if ((flags & WindowFlags::Features::VSYNC) > 0) resetFlags |= BGFX_RESET_VSYNC;
		if ((flags & WindowFlags::Features::MSAA) > 0) resetFlags |= BGFX_RESET_MAXANISOTROPY;

		init.resolution.reset = resetFlags;
		init.platformData.nwh = glfwNativeWindowHandle(GLFWHANDLE);
		init.platformData.ndt = getNativeDisplayHandle();

		if (!bgfx::init(init)) throw std::runtime_error("[RawrBox-Render] Failed to initialize bgfx");

		auto debugFlags = BGFX_DEBUG_NONE;
		if ((flags & WindowFlags::Debug::WIREFRAME) > 0) debugFlags |= BGFX_DEBUG_WIREFRAME;
		if ((flags & WindowFlags::Debug::STATS) > 0) debugFlags |= BGFX_DEBUG_STATS;
		if ((flags & WindowFlags::Debug::TEXT) > 0) debugFlags |= BGFX_DEBUG_TEXT;

		bgfx::setDebug(debugFlags);

		glfwSetKeyCallback(GLFWHANDLE, callbacks_key);
		glfwSetCharCallback(GLFWHANDLE, callbacks_char);
		glfwSetScrollCallback(GLFWHANDLE, callbacks_scroll);
		glfwSetWindowSizeCallback(GLFWHANDLE, callbacks_resize);
		glfwSetWindowFocusCallback(GLFWHANDLE, callbacks_focus);
		glfwSetCursorPosCallback(GLFWHANDLE, callbacks_mouseMove);
		glfwSetMouseButtonCallback(GLFWHANDLE, callbacks_mouseKey);
		glfwSetWindowCloseCallback(GLFWHANDLE, callbacks_windowClose);
	}

	void Window::setMonitor(int monitor) {
		this->_monitor = monitor;
	}

	void Window::setRenderer(bgfx::RendererType::Enum render) {
		if (!this->isRendererSupported(render)) throw std::runtime_error(fmt::format("[RawrBox-Render] Window {} is not supported by your OS", bgfx::getRendererName(render)));
		this->_renderType = render;
	}

	void Window::setTitle(const std::string& title) {
		this->_title = title;
	}

	void Window::shutdown() {
		bgfx::shutdown();
		glfwTerminate();
	}

	void Window::pollEvents() {
		if (this->_handle == nullptr) return;
		glfwPollEvents();
	}

	// ------UTILS
	void Window::close() {
		if (this->_handle == nullptr) return;
		glfwDestroyWindow(GLFWHANDLE);
		this->_handle = nullptr;
	}

	bool Window::isRendererSupported(bgfx::RendererType::Enum render) {
		if (render == bgfx::RendererType::Count) return true;

		bgfx::RendererType::Enum supportedRenderers[bgfx::RendererType::Count];
		uint8_t num = bgfx::getSupportedRenderers(BX_COUNTOF(supportedRenderers), supportedRenderers);
		for (uint8_t i = 0; i < num; ++i) {
			if (supportedRenderers[i] == render) return true;
		}

		return false;
	}

	Vector2i Window::getSize() const {
		if (this->_handle == nullptr) throw std::runtime_error("[RawrBox-Render] Window not initialized, handle not found");
		Vector2i ret;
		glfwGetWindowSize(GLFWHANDLE, &ret.x, &ret.y);

		return ret;
	}

	Vector2i Window::getMousePos() const {
		if (this->_handle == nullptr) throw std::runtime_error("[RawrBox-Render] Window not initialized, handle not found");
		double x, y;

		glfwGetCursorPos(GLFWHANDLE, &x, &y);
		return Vector2i(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)));
	}

	bool Window::getShouldClose() const {
		if (this->_handle == nullptr) return false;
		return glfwWindowShouldClose(GLFWHANDLE);
	}

	void Window::setShouldClose(bool close) const {
		if (this->_handle == nullptr) return;
		glfwSetWindowShouldClose(GLFWHANDLE, close ? 1 : 0);
	}

	bool Window::isKeyDown(int key) {
		auto fnd = this->keysIn.find(key);
		if (fnd == this->keysIn.end()) return false;
		return fnd->second;
	}
	// --------------------

	// ------EVENTS
	void Window::callbacks_windowClose(GLFWwindow* whandle) {
		auto& window = glfwHandleToRenderer(whandle);
		window.onWindowClose(window);
	}

	void Window::callbacks_resize(GLFWwindow* whandle, int width, int height) {
		auto& window = glfwHandleToRenderer(whandle);

		bgfx::reset(static_cast<uint32_t>(width), static_cast<uint32_t>(height), BGFX_RESET_VSYNC);
		window.onResize(window, {width, height});
	}

	void Window::callbacks_key(GLFWwindow* whandle, int key, int scancode, int action, int mods) {
		if (key < 0) return;

		auto& window = glfwHandleToRenderer(whandle);
		window.onKey(window,
		    static_cast<unsigned int>(key),
		    static_cast<unsigned int>(scancode),
		    static_cast<unsigned int>(action),
		    static_cast<unsigned int>(mods));

		if (action == GLFW_REPEAT) return;
		window.keysIn[key] = action != GLFW_RELEASE ? 1 : 0;
	}

	void Window::callbacks_mouseKey(GLFWwindow* whandle, int button, int action, int mods) {
		if (button < 0) return;

		auto& window = glfwHandleToRenderer(whandle);
		auto pos = window.getMousePos();

		window.onMouseKey(window,
		    pos,
		    static_cast<unsigned int>(button),
		    static_cast<unsigned int>(action),
		    static_cast<unsigned int>(mods));

		window.mouseIn[button] = action == GLFW_PRESS ? 1 : 0;
	}

	void Window::callbacks_char(GLFWwindow* whandle, unsigned int ch) {
		auto& window = glfwHandleToRenderer(whandle);
		window.onChar(window, ch);
	}

	void Window::callbacks_scroll(GLFWwindow* whandle, double x, double y) {
		auto& window = glfwHandleToRenderer(whandle);

		auto size = window.getSize();
		auto pos = window.getMousePos();

		if (pos.x < 0 || pos.y < 0 || pos.x > size.x || pos.y > size.y) return; // Outside window
		window.onScroll(window, pos, {static_cast<int>(x * 10), static_cast<int>(y * 10)});
	}

	void Window::callbacks_mouseMove(GLFWwindow* whandle, double x, double y) {
		auto& window = glfwHandleToRenderer(whandle);
		window.onMouseMove(window, {static_cast<int>(x), static_cast<int>(y)});
	}

	void Window::callbacks_focus(GLFWwindow* whandle, int focus) {
		auto& window = glfwHandleToRenderer(whandle);

		window.hasFocus = focus == 1;
		window.onFocus(window, focus);
	}
	// --------------------

	Window::~Window() { this->close(); }
} // namespace rawrBox
