
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/window.hpp>

#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <cmath>

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
#include <fmt/printf.h>

#include <map>
#include <stdexcept>

#define GLFWHANDLE (std::bit_cast<GLFWwindow*>(_handle))
#define GLFWCURSOR (std::bit_cast<GLFWcursor*>(_cursor))

namespace rawrbox {
	// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
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
	// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

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

		if (!fullscreen && !windowed && !borderless) throw std::runtime_error("[RawrBox-Window] Window flag attribute missing");
		if (windowed && (borderless || fullscreen)) throw std::runtime_error("[RawrBox-Window] Only one window flag attribute can be selected");
		if (borderless && (windowed || fullscreen)) throw std::runtime_error("[RawrBox-Window] Only one window flag attribute can be selected");
		if (fullscreen && (windowed || borderless)) throw std::runtime_error("[RawrBox-Window] Only one window aflag ttribute can be selected");

		if (fullscreen || (width >= mode->width || height >= mode->height)) {
			width = mode->width;
			height = mode->height;
		} else if (borderless) {
			width = mode->width - 1;
			height = mode->height - 1;
		}
		// -----------

		// Set transparent
		bool transparent = (flags & WindowFlags::Features::TRANSPARENT_BUFFER) > 0;
		if (transparent) {
			glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
		}
		// ----

		// Window properties
		properties[GLFW_RESIZABLE] = (flags & WindowFlags::Features::RESIZABLE) > 0 ? 1 : 0;
		properties[GLFW_DECORATED] = borderless || fullscreen || transparent ? 0 : 1;
		for (const auto& pair : properties) {
			glfwWindowHint(pair.first, pair.second);
		}
		// ------

		auto glfwHandle = glfwCreateWindow(width, height, this->_title.c_str(), windowed || borderless ? nullptr : mon, nullptr);
		if (glfwHandle == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Window] Failed to initialize window [{} - {}x{}]", this->_title, width, height));

		this->_handle = glfwHandle;
		glfwSetWindowUserPointer(glfwHandle, this);

		// Create cursor
		this->setCursor(GLFW_ARROW_CURSOR);
		// -------------

		// Center window
		if (windowed || transparent) {
			int monx = 0, mony = 0;
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
		::SendMessage(hwnd, WM_SETICON, ICON_SMALL, std::bit_cast<LPARAM>(hIcon));
		::SendMessage(hwnd, WM_SETICON, ICON_BIG, std::bit_cast<LPARAM>(hIcon));
#endif
		// ---------------

		if ((flags & WindowFlags::Features::MULTI_THREADED) == 0) bgfx::renderFrame(); // Disable multi-threading

		bgfx::Init init;
		if (this->_renderType == bgfx::RendererType::Vulkan && (flags & WindowFlags::Features::MULTI_THREADED) == 0) {
			fmt::print("[RawrBox-Window] WARNING: VULKAN SHOULD HAVE THE 'WindowFlags::Features::MULTI_THREADED' SET FOR BETTER PERFORMANCE!\n");
		}

		init.type = this->_renderType;
		init.resolution.width = static_cast<uint32_t>(width);
		init.resolution.height = static_cast<uint32_t>(height);

		this->_resetFlags = BGFX_RESET_NONE;
		if ((flags & WindowFlags::Features::VSYNC) > 0) this->_resetFlags |= BGFX_RESET_VSYNC;
		if ((flags & WindowFlags::Features::MSAA) > 0) this->_resetFlags |= BGFX_RESET_MAXANISOTROPY;
		if (transparent) this->_resetFlags |= BGFX_RESET_TRANSPARENT_BACKBUFFER;

		init.resolution.reset = this->_resetFlags;
		init.platformData.nwh = glfwNativeWindowHandle(GLFWHANDLE);
		init.platformData.ndt = getNativeDisplayHandle();

		if (!bgfx::init(init)) throw std::runtime_error("[RawrBox-Render] Failed to initialize bgfx");
		rawrbox::BGFX_INITIALIZED = true;

		if ((flags & WindowFlags::Debug::WIREFRAME) > 0) this->_debugFlags |= BGFX_DEBUG_WIREFRAME;
		if ((flags & WindowFlags::Debug::STATS) > 0) this->_debugFlags |= BGFX_DEBUG_STATS;
		if ((flags & WindowFlags::Debug::TEXT) > 0) this->_debugFlags |= BGFX_DEBUG_TEXT;
		if ((flags & WindowFlags::Debug::PROFILER) > 0) this->_debugFlags |= BGFX_DEBUG_PROFILER;

		bgfx::setDebug(this->_debugFlags);

		glfwSetKeyCallback(GLFWHANDLE, callbacks_key);
		glfwSetCharCallback(GLFWHANDLE, callbacks_char);
		glfwSetScrollCallback(GLFWHANDLE, callbacks_scroll);
		glfwSetWindowSizeCallback(GLFWHANDLE, callbacks_resize);
		glfwSetWindowFocusCallback(GLFWHANDLE, callbacks_focus);
		glfwSetCursorPosCallback(GLFWHANDLE, callbacks_mouseMove);
		glfwSetMouseButtonCallback(GLFWHANDLE, callbacks_mouseKey);
		glfwSetWindowCloseCallback(GLFWHANDLE, callbacks_windowClose);

		// Setup main renderer ----
		this->_stencil = std::make_unique<rawrbox::Stencil>(this->getSize());
		this->_renderer = std::make_unique<rawrbox::Renderer>(0, this->getSize());
		this->onResize += [this](auto&, auto& size) {
			if (this->_renderer != nullptr) this->_renderer->resizeView(size);
			if (this->_stencil != nullptr) this->_stencil->resize(size);
		};

		// Setup global util textures ---
		if (rawrbox::__OPEN_WINDOWS__ == 0) {
			if (rawrbox::MISSING_TEXTURE == nullptr)
				rawrbox::MISSING_TEXTURE = std::make_shared<rawrbox::TextureMissing>();

			if (rawrbox::WHITE_TEXTURE == nullptr)
				rawrbox::WHITE_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Colors::White);

			if (rawrbox::MISSING_SPECULAR_EMISSIVE_TEXTURE == nullptr)
				rawrbox::MISSING_SPECULAR_EMISSIVE_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Colors::Black);
		}
		// ------------------

		rawrbox::__OPEN_WINDOWS__++;
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

	void Window::setClearColor(uint32_t clearColor) {
		if (this->_renderer == nullptr) return;
		this->_renderer->setClearColor(clearColor);
	}

	// CURSOR ------
	void Window::hideCursor(bool hidden) {
		if (GLFWHANDLE == nullptr) return;
		glfwSetInputMode(GLFWHANDLE, GLFW_CURSOR, hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
	}

	void Window::setCursor(uint32_t icon) {
		if (GLFWCURSOR != nullptr) glfwDestroyCursor(GLFWCURSOR); // Delete old one

		auto cursor = glfwCreateStandardCursor(icon);
		this->_cursor = cursor;

		glfwSetCursor(GLFWHANDLE, cursor);
	}

	void Window::setCursor(const std::array<uint8_t, 1024>& pixels) {
		GLFWimage image = {};
		image.width = 16;
		image.height = 16;

		std::memcpy(image.pixels, pixels.data(), pixels.size() * sizeof(uint8_t));

		if (GLFWCURSOR != nullptr) glfwDestroyCursor(GLFWCURSOR); // Delete old one
		auto cursor = glfwCreateCursor(&image, 0, 0);
		this->_cursor = cursor;

		glfwSetCursor(GLFWHANDLE, cursor);
	}
	// -------------------

	void Window::shutdown() {
		bgfx::shutdown();
		glfwTerminate();
	}

	// UPDATE ----
	void Window::pollEvents() {
		if (this->_handle == nullptr) return;
		glfwPollEvents();
	}

	void Window::update() {}
	// ------

	// DRAW ------
	void Window::clear() {
		if (this->_renderer == nullptr) return;
		this->_renderer->clear(); // Clean up and set renderer
	}

	void Window::upload() {
		if (this->_renderer == nullptr) return;

		// MISSING TEXTURES ------
		rawrbox::MISSING_TEXTURE->upload();
		rawrbox::WHITE_TEXTURE->upload();
		rawrbox::MISSING_SPECULAR_EMISSIVE_TEXTURE->upload();
		// ------------------

		this->_stencil->upload();
		// -----
	}

	void Window::frame() const {
		if (this->_renderer == nullptr) return;
		bgfx::frame();
	}

	// -------------------

	// ------UTILS
	void Window::close() {
		if (rawrbox::__OPEN_WINDOWS__-- <= 0) {
			rawrbox::MISSING_TEXTURE = nullptr;
			rawrbox::WHITE_TEXTURE = nullptr;
			rawrbox::MISSING_SPECULAR_EMISSIVE_TEXTURE = nullptr;
		}

		this->_stencil = nullptr;
		this->_renderer = nullptr;

		if (GLFWHANDLE != nullptr) glfwDestroyWindow(GLFWHANDLE);
		this->_handle = nullptr;
		if (GLFWCURSOR != nullptr) glfwDestroyCursor(GLFWCURSOR);
		this->_cursor = nullptr;
	}

	bool Window::isRendererSupported(bgfx::RendererType::Enum render) {
		if (render == bgfx::RendererType::Count) return true;

		// NOLINTBEGIN(hicpp-avoid-c-arrays)
		bgfx::RendererType::Enum supportedRenderers[bgfx::RendererType::Count];
		uint8_t num = bgfx::getSupportedRenderers(BX_COUNTOF(supportedRenderers), supportedRenderers);
		for (uint8_t i = 0; i < num; ++i) {
			if (supportedRenderers[i] == render) return true;
		}
		// NOLINTEND(hicpp-avoid-c-arrays)

		return false;
	}

	Vector2i Window::getSize() const {
		if (this->_handle == nullptr) throw std::runtime_error("[RawrBox-Render] Window not initialized, handle not found");
		Vector2i ret;
		glfwGetWindowSize(GLFWHANDLE, &ret.x, &ret.y);

		return ret;
	}

	float Window::getAspectRatio() const {
		if (this->_handle == nullptr) throw std::runtime_error("[RawrBox-Render] Window not initialized, handle not found");
		Vector2f ret = this->getSize().cast<float>();

		return ret.x / ret.y;
	}

	Vector2i Window::getMousePos() const {
		if (this->_handle == nullptr) throw std::runtime_error("[RawrBox-Render] Window not initialized, handle not found");
		double x = NAN, y = NAN;

		glfwGetCursorPos(GLFWHANDLE, &x, &y);
		return {static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y))};
	}

	bool Window::getShouldClose() const {
		if (this->_handle == nullptr) return false;
		return glfwWindowShouldClose(GLFWHANDLE);
	}

	void Window::setShouldClose(bool close) const {
		if (this->_handle == nullptr) return;
		glfwSetWindowShouldClose(GLFWHANDLE, close ? 1 : 0);
	}

	bool Window::isKeyDown(int key) const {
		auto fnd = this->keysIn.find(key);
		if (fnd == this->keysIn.end()) return false;
		return fnd->second;
	}

	rawrbox::Stencil& Window::getStencil() const { return *this->_stencil; }
	// --------------------

	// ------EVENTS
	void Window::callbacks_windowClose(GLFWwindow* whandle) {
		auto& window = glfwHandleToRenderer(whandle);
		window.onWindowClose(window);
	}

	void Window::callbacks_resize(GLFWwindow* whandle, int width, int height) {
		auto& window = glfwHandleToRenderer(whandle);

		bgfx::reset(static_cast<uint32_t>(width), static_cast<uint32_t>(height), window._resetFlags);
		window.onResize(window, {width, height});
	}

	void Window::callbacks_key(GLFWwindow* whandle, int key, int scancode, int action, int mods) {
		if (key < 0) return;

		auto& window = glfwHandleToRenderer(whandle);
		window.onKey(window,
		    static_cast<uint32_t>(key),
		    static_cast<uint32_t>(scancode),
		    static_cast<uint32_t>(action),
		    static_cast<uint32_t>(mods));

		if (action == GLFW_REPEAT) return;
		window.keysIn[key] = action != GLFW_RELEASE ? 1 : 0;
	}

	void Window::callbacks_mouseKey(GLFWwindow* whandle, int button, int action, int mods) {
		if (button < 0) return;

		auto& window = glfwHandleToRenderer(whandle);
		auto pos = window.getMousePos();

		window.onMouseKey(window,
		    pos,
		    static_cast<uint32_t>(button),
		    static_cast<uint32_t>(action),
		    static_cast<uint32_t>(mods));

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
		window.onMouseScroll(window, pos, {static_cast<int>(x * 10), static_cast<int>(y * 10)});
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
} // namespace rawrbox
