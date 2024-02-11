

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
	#error "GLFW 3.2 or later is required"
#endif // GLFW_VERSION_MINOR < 2

#if PLATFORM_LINUX
	#if RAWRBOX_USE_WAYLAND
		#include <wayland-egl.h>
		#define GLFW_EXPOSE_NATIVE_WAYLAND
	#else
		#define GLFW_EXPOSE_NATIVE_X11
		#define GLFW_EXPOSE_NATIVE_GLX
	#endif
#elif PLATFORM_MACOS
	#define GLFW_EXPOSE_NATIVE_COCOA
	#define GLFW_EXPOSE_NATIVE_NSGL
#elif PLATFORM_WIN32
	#define GLFW_EXPOSE_NATIVE_WIN32
	#define GLFW_EXPOSE_NATIVE_WGL
#endif

#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/utils/string.hpp>

#include <magic_enum.hpp>

#include <GLFW/glfw3native.h>

namespace rawrbox {
	// PRIVATE -----

	// STATIC -----
	std::unique_ptr<rawrbox::Logger> Window::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Window");

	std::vector<std::unique_ptr<rawrbox::Window>> Window::__WINDOWS = {};
	Diligent::RENDER_DEVICE_TYPE Window::__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_UNDEFINED;
	// -------------

	// ---------------

	rawrbox::Window* Window::createWindow(Diligent::RENDER_DEVICE_TYPE render) {
		bool autoDetermine = render == Diligent::RENDER_DEVICE_TYPE_UNDEFINED || render == Diligent::RENDER_DEVICE_TYPE_COUNT;
		if (autoDetermine) {
			Window::__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_COUNT;

			// Prefer newest API's first
#if PLATFORM_LINUX
	#if RAWRBOX_SUPPORT_VULKAN
			Window::__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_VULKAN;
	#else
			Window::__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_GL;
	#endif
#else
	#if RAWRBOX_SUPPORT_VULKAN
			Window::__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_VULKAN;
	#elif RAWRBOX_SUPPORT_DX12
			Window::__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_D3D12;
	#elif RAWRBOX_SUPPORT_DX11
			Window::__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_D3D11;
	#else
			Window::__RENDER_TYPE = Diligent::RENDER_DEVICE_TYPE_GL;
	#endif
#endif
			if (Window::__RENDER_TYPE == Diligent::RENDER_DEVICE_TYPE_COUNT) {
				throw _logger->error("Failed to automatically determine best renderer type");
			}
		} else {
			Window::__RENDER_TYPE = render;
		}

		if (render == Diligent::RENDER_DEVICE_TYPE_GL || render == Diligent::RENDER_DEVICE_TYPE_GLES) {
			rawrbox::Matrix4x4::MTX_RIGHT_HANDED = true;
		}

		Window::__WINDOWS.push_back(std::unique_ptr<rawrbox::Window>(new Window(Window::__RENDER_TYPE)));
		return Window::__WINDOWS.front().get();
	}

	rawrbox::Window* Window::getWindow(size_t indx) {
		if (indx > Window::__WINDOWS.size()) throw _logger->error("Invalid window index '{}'", indx);
		return Window::__WINDOWS[indx].get();
	}

	void Window::pollEvents() {
		if (__WINDOWS.empty()) return;
		glfwWaitEvents();
	}

	void Window::shutdown() {
		__WINDOWS.clear();
		_logger.reset();

		// SHUTDOWN FONTS ----
		rawrbox::TextEngine::shutdown();

		rawrbox::DEBUG_FONT_REGULAR = nullptr;
		rawrbox::DEBUG_FONT_BOLD = nullptr;
		rawrbox::DEBUG_FONT_ITALIC = nullptr;
		// --------------------

		// SHUTDOWN TEXTURES ----
		rawrbox::MISSING_TEXTURE.reset();
		rawrbox::MISSING_VERTEX_TEXTURE.reset();

		rawrbox::WHITE_TEXTURE.reset();
		rawrbox::BLACK_TEXTURE.reset();
		rawrbox::NORMAL_TEXTURE.reset();
		// ----------------------

		glfwPostEmptyEvent();
		glfwTerminate();
	}

	void Window::update() {
		for (auto& win : __WINDOWS) {
			if (win->_renderer == nullptr) continue;

			setActiveRenderer(win->_renderer.get());
			win->_renderer->update();
		}
	}

	void Window::render() {
		for (auto& win : __WINDOWS) {
			if (win->_renderer == nullptr) continue;

			setActiveRenderer(win->_renderer.get());
			win->_renderer->render();
		}
	}

	void Window::setActiveRenderer(rawrbox::RendererBase* r) {
		rawrbox::RENDERER = r;
	}

	// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
	static Window& glfwHandleToRenderer(GLFWwindow* ptr) {
		return *static_cast<Window*>(glfwGetWindowUserPointer(ptr));
	}

	static void glfw_errorCallback(int error, const char* description) {
		fmt::print("[RawrBox-Window] GLFW error {}: {}\n", error, description);
	}
	// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
	// --------------------------

	Window::~Window() { this->close(); }
	Window::Window(Diligent::RENDER_DEVICE_TYPE type) : _renderType(type) {
		int count = 0;
		auto monitors = glfwGetMonitors(&count);

		for (int i = 0; i < count; i++) {
			const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
			this->_screenSizes[i] = {mode->width, mode->height};
		}
	}

	void Window::init(int width, int height, uint32_t flags) {
		if (rawrbox::RENDER_THREAD_ID == std::this_thread::get_id()) throw _logger->error("'init' should be called inside engine's 'setupGLFW'!");

		int APIHint = GLFW_NO_API;
#ifndef _WIN32
		if (this->_renderType == Diligent::RENDER_DEVICE_TYPE_GL) {
			// On platforms other than Windows Diligent Engine
			// attaches to existing OpenGL context
			APIHint = GLFW_OPENGL_API;
		}
#endif

		glfwSetErrorCallback(glfw_errorCallback);
		if (!glfwInit()) throw _logger->error("Failed to initialize glfw");

		glfwWindowHint(GLFW_CLIENT_API, APIHint); // Disable opengl
		if (APIHint == GLFW_OPENGL_API) {
			// We need compute shaders, so request OpenGL 4.2 at least
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		}
		// -------------

		std::unordered_map<int, int> properties = {};

		// Monitor selection ----
		GLFWmonitor* mon = glfwGetPrimaryMonitor();
		if (this->_settings.monitor >= 0) {
			int monitorscount = 0;
			auto monitors = glfwGetMonitors(&monitorscount);
			if (this->_settings.monitor < monitorscount) {
				mon = monitors[this->_settings.monitor];
				properties[GLFW_VISIBLE] = 0;
			}
		}

		if (mon == nullptr) throw _logger->error("Failed to get primary window");
		// ----------------------------

		// Fullscreen / borderless
		const GLFWvidmode* mode = glfwGetVideoMode(mon);
		bool windowed = (flags & WindowFlags::Window::WINDOWED) > 0;
		bool borderless = (flags & WindowFlags::Window::BORDERLESS) > 0;
		bool fullscreen = (flags & WindowFlags::Window::FULLSCREEN) > 0;

		if (!fullscreen && !windowed && !borderless) throw _logger->error("Window flag attribute missing");
		if (windowed && (borderless || fullscreen)) throw _logger->error("Only one window flag attribute can be selected");
		if (borderless && (windowed || fullscreen)) throw _logger->error("Only one window flag attribute can be selected");
		if (fullscreen && (windowed || borderless)) throw _logger->error("Only one window flag attribute can be selected");

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

#ifdef _DEBUG
		std::string renderTypeName = std::string(magic_enum::enum_name(this->_renderType));
		this->_settings.title += fmt::format(" - {}", rawrbox::StrUtils::replace(renderTypeName, "RENDER_DEVICE_TYPE_", ""));
#endif

		auto glfwHandle = glfwCreateWindow(width, height, this->_settings.title.c_str(), windowed || borderless ? nullptr : mon, nullptr);
		if (glfwHandle == nullptr) throw _logger->error("Failed to initialize window [{} - {}x{}]", this->_settings.title, width, height);

		this->_handle = glfwHandle;
		glfwSetWindowUserPointer(glfwHandle, this);

		// Create cursor
		this->setCursor(GLFW_ARROW_CURSOR);
		// -------------

		// Center window
		if (this->_settings.pos == -1) {
			if (windowed || transparent) {
				int monx = 0, mony = 0;
				glfwGetMonitorPos(mon, &monx, &mony);

				this->_settings.pos = {monx + mode->width / 2 - width / 2, mony + mode->height / 2 - height / 2}; // Center
				glfwSetWindowPos(glfwHandle, this->_settings.pos.x, this->_settings.pos.y);

				glfwShowWindow(glfwHandle);
			}
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

		glfwSetKeyCallback(this->_handle, callbacks_key);
		glfwSetCharCallback(this->_handle, callbacks_char);
		glfwSetScrollCallback(this->_handle, callbacks_scroll);
		glfwSetWindowSizeCallback(this->_handle, callbacks_resize);
		glfwSetWindowFocusCallback(this->_handle, callbacks_focus);
		glfwSetWindowPosCallback(this->_handle, callbacks_pos);
		glfwSetCursorPosCallback(this->_handle, callbacks_mouseMove);
		glfwSetMouseButtonCallback(this->_handle, callbacks_mouseKey);
		glfwSetWindowCloseCallback(this->_handle, callbacks_windowClose);

		// Initialize renderer
		this->_settings.flags = flags;
		this->_settings.size = {width, height};
		// -------------------
	}

	void Window::setMonitor(int monitor) {
		this->_settings.monitor = monitor;
	}

	void Window::setTitle(const std::string& title) {
		this->_settings.title = title;
	}

	// CURSOR ------
	void Window::hideCursor(bool hidden) {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");
		glfwSetInputMode(this->_handle, GLFW_CURSOR, hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
	}

	void Window::setCursor(uint32_t icon) {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");
		if (this->_cursor != nullptr) glfwDestroyCursor(this->_cursor); // Delete old one
		auto cursor = glfwCreateStandardCursor(icon);
		this->_cursor = cursor;

		glfwSetCursor(this->_handle, cursor);
	}

	void Window::setCursor(const std::array<uint8_t, 1024>& pixels) {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");
		if (pixels.empty()) throw _logger->error("Cursor pixels cannot be empty");

		std::memcpy(this->_cursorPixels.data(), pixels.data(), pixels.size() * sizeof(uint8_t));
		if (this->_cursor != nullptr) glfwDestroyCursor(this->_cursor); // Delete old one

		GLFWimage image = {};
		image.pixels = this->_cursorPixels.data();
		image.width = 16;
		image.height = 16;

		auto cursor = glfwCreateCursor(&image, 0, 0);
		this->_cursor = cursor;

		glfwSetCursor(this->_handle, cursor);
	}
	// -------------------

	// ------UTILS
	void Window::close() {
		if (this->_handle != nullptr) {
			glfwDestroyWindow(this->_handle);
			this->_handle = nullptr;
		}

		if (this->_cursor != nullptr) {
			glfwDestroyCursor(this->_cursor);
			this->_cursor = nullptr;
		}
	}

	rawrbox::Vector2i Window::getSize() const {
		return this->_settings.size;
	}

	void Window::setPos(const rawrbox::Vector2i& pos) {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");

		glfwSetWindowPos(this->_handle, pos.x, pos.y);
		this->_settings.pos = pos;
	}

	rawrbox::Vector2i Window::getPos() const {
		return this->_settings.pos;
	}

	rawrbox::Vector2i Window::getMonitorSize() const {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");

		GLFWmonitor* w = getWindowMonitor();
		if (w == nullptr) throw _logger->error("Failed to find screen dimensions");

		auto vidmode = glfwGetVideoMode(w);
		return {vidmode->width, vidmode->height};
	}

	float Window::getAspectRatio() const {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");
		Vector2f ret = this->getSize().cast<float>();

		return ret.x / ret.y;
	}

	Vector2i Window::getMousePos() const {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");
		double x = NAN, y = NAN;

		glfwGetCursorPos(this->_handle, &x, &y);
		return {static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y))};
	}

	uint32_t Window::getWindowFlags() const {
		return this->_settings.flags;
	}

	Diligent::NativeWindow Window::getHandle() const {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");
			// Get native window ----
#if PLATFORM_WIN32
		Diligent::Win32NativeWindow window{glfwGetWin32Window(this->_handle)};
#endif

#if PLATFORM_LINUX
		Diligent::LinuxNativeWindow window;
		window.WindowId = glfwGetX11Window(this->_handle);
		window.pDisplay = glfwGetX11Display();
		if (this->_renderType == Diligent::RENDER_DEVICE_TYPE_GL)
			glfwMakeContextCurrent(this->_handle);
#endif

#if PLATFORM_MACOS
		Diligent::MacOSNativeWindow window;
		if (this->_renderType == Diligent::RENDER_DEVICE_TYPE_GL)
			glfwMakeContextCurrent(this->_handle);
		else
			window.pNSView = GetNSWindowView(this->_handle);
#endif
		// ------------
		return window;
	}

	rawrbox::RendererBase& Window::getRenderer() const { return *this->_renderer.get(); }

	bool Window::isKeyDown(int key) const {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");
		return glfwGetKey(this->_handle, key) == GLFW_PRESS;
	}

	bool Window::isMouseDown(int key) const {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");
		return glfwGetMouseButton(this->_handle, key) == GLFW_PRESS;
	}

	const std::unordered_map<int, rawrbox::Vector2i>& Window::getScreenSizes() const {
		return this->_screenSizes;
	}

	bool Window::hasFocus() const {
		return this->_hasFocus;
	}
	// --------------------

	// ------EVENTS
	void Window::callbacks_windowClose(GLFWwindow* whandle) {
		auto& window = glfwHandleToRenderer(whandle);

		glfwSetWindowShouldClose(whandle, true);
		glfwPostEmptyEvent();

		window.onWindowClose(window);
	}

	void Window::callbacks_resize(GLFWwindow* whandle, int width, int height) {
		rawrbox::runOnRenderThread([whandle, width, height]() {
			auto& window = glfwHandleToRenderer(whandle);
			window._settings.size = {width, height};

			window.onResize(window, window._settings.size, window.getMonitorSize());
		});
	}

	void Window::callbacks_key(GLFWwindow* whandle, int key, int scancode, int action, int mods) {
		if (key < 0) return;

		rawrbox::runOnRenderThread([whandle, key, scancode, action, mods]() {
			auto& window = glfwHandleToRenderer(whandle);

			window.onKey(window,
			    static_cast<uint32_t>(key),
			    static_cast<uint32_t>(scancode),
			    static_cast<uint32_t>(action),
			    static_cast<uint32_t>(mods));
		});
	}

	void Window::callbacks_mouseKey(GLFWwindow* whandle, int button, int action, int mods) {
		if (button < 0) return;

		rawrbox::runOnRenderThread([whandle, button, action, mods]() {
			auto& window = glfwHandleToRenderer(whandle);
			auto pos = window.getMousePos();

			window.onMouseKey(window,
			    pos,
			    static_cast<uint32_t>(button),
			    static_cast<uint32_t>(action),
			    static_cast<uint32_t>(mods));
		});
	}

	void Window::callbacks_char(GLFWwindow* whandle, unsigned int ch) {
		rawrbox::runOnRenderThread([whandle, ch]() {
			auto& window = glfwHandleToRenderer(whandle);
			window.onChar(window, ch);
		});
	}

	void Window::callbacks_scroll(GLFWwindow* whandle, double x, double y) {
		rawrbox::runOnRenderThread([whandle, x, y]() {
			auto& window = glfwHandleToRenderer(whandle);

			auto size = window.getSize();
			auto pos = window.getMousePos();

			if (pos.x < 0 || pos.y < 0 || pos.x > size.x || pos.y > size.y) return; // Outside window
			window.onMouseScroll(window, pos, {static_cast<int>(x * 10), static_cast<int>(y * 10)});
		});
	}

	void Window::callbacks_mouseMove(GLFWwindow* whandle, double x, double y) {
		rawrbox::runOnRenderThread([whandle, x, y]() {
			auto& window = glfwHandleToRenderer(whandle);
			window.onMouseMove(window, {static_cast<int>(x), static_cast<int>(y)});
		});
	}

	void Window::callbacks_focus(GLFWwindow* whandle, int focus) {
		rawrbox::runOnRenderThread([whandle, focus]() {
			auto& window = glfwHandleToRenderer(whandle);

			window._hasFocus = (focus == 1);
			window.onFocus(window, focus);
		});
	}

	void Window::callbacks_pos(GLFWwindow* whandle, int x, int y) {
		rawrbox::runOnRenderThread([whandle, x, y]() {
			auto& window = glfwHandleToRenderer(whandle);

			window._settings.pos = {x, y};
			window.onWindowMove(window, window._settings.pos);
		});
	}

	// Adapted from : https://github.com/glfw/glfw/pull/2220/files
	GLFWmonitor* Window::getWindowMonitor() const {
		if (this->_handle == nullptr) throw _logger->error("Invalid window handle");

		int monitorCount = 0;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
		if (monitorCount == 1) return monitors[0];

		GLFWmonitor* result = nullptr;
		const GLFWvidmode* vidmode = nullptr;

		rawrbox::AABBi windowRect = {};
		glfwGetWindowPos(this->_handle, &windowRect.pos.x, &windowRect.pos.y);
		glfwGetWindowSize(this->_handle, &windowRect.size.x, &windowRect.size.y);

		rawrbox::AABBi scratchRect = {};
		rawrbox::AABBi overlapRect = {};
		glfwGetWindowFrameSize(this->_handle, &scratchRect.pos.x, &scratchRect.pos.y,
		    &scratchRect.size.x, &scratchRect.size.y);

		windowRect.pos.x -= scratchRect.pos.x;
		windowRect.pos.y -= scratchRect.pos.y;
		windowRect.size.x += scratchRect.pos.x + scratchRect.size.x;
		windowRect.size.y += scratchRect.pos.y + scratchRect.size.y;

		size_t overlapMonitor = 0;
		auto totalMonitors = static_cast<size_t>(monitorCount);
		uint32_t currentDim = 0, overlapDim = 0;

		for (size_t i = 0; i < totalMonitors; i++) {
			rawrbox::AABBi monitorRect = {};
			glfwGetMonitorPos(monitors[i], &monitorRect.pos.x, &monitorRect.pos.y);

			vidmode = glfwGetVideoMode(monitors[i]);
			monitorRect.size.x = vidmode->width;
			monitorRect.size.y = vidmode->height;

			scratchRect = rawrbox::AABBi::intersects(windowRect, monitorRect);

			currentDim = scratchRect.size.x * scratchRect.size.y;
			overlapDim = overlapRect.size.x * overlapRect.size.y;

			if (currentDim > 0 && currentDim > overlapDim) {
				overlapRect = scratchRect;
				overlapMonitor = i;
			}
		}

		if (overlapMonitor >= 0) result = monitors[overlapMonitor];
		return result;
	}
	// --------------------

} // namespace rawrbox
