#include <rawrbox/render/renderer.h>

#include <bx/bx.h>
#include <bgfx/bgfx.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
#	error "GLFW 3.2 or later is required"
#endif // GLFW_VERSION_MINOR < 2

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#	if RAWRBOX_USE_WAYLAND
#		include <wayland-egl.h>
#		define GLFW_EXPOSE_NATIVE_WAYLAND
#	else
#		define GLFW_EXPOSE_NATIVE_X11
#		define GLFW_EXPOSE_NATIVE_GLX
#	endif
#elif BX_PLATFORM_OSX
#	define GLFW_EXPOSE_NATIVE_COCOA
#	define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#endif //
#include <GLFW/glfw3native.h>

#include <fmt/printf.h>
#include <stdexcept>

#define GLFWHANDLE reinterpret_cast<GLFWwindow*>(_handle)

namespace rawrBOX {
	static Renderer& glfwHandleToRenderer(GLFWwindow* ptr) {
		return *static_cast<Renderer*>(glfwGetWindowUserPointer(ptr));
	}

	static void glfw_errorCallback(int error, const char *description) {
		fmt::print("GLFW error {}: {}\n", error, description);
	}

	static void* glfwNativeWindowHandle(GLFWwindow* _window) {
	#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	# 		if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
				wl_egl_window *win_impl = static_cast<wl_egl_window*>(glfwGetWindowUserPointer(_window));
				if(!win_impl)
				{
					int width, height;
					glfwGetWindowSize(_window, &width, &height);
					struct wl_surface* surface = (struct wl_surface*)glfwGetWaylandWindow(_window);
					if(!surface)
						return nullptr;
					win_impl = wl_egl_window_create(surface, width, height);
					glfwSetWindowUserPointer(_window, (void*)(uintptr_t)win_impl);
				}

				return (void*)(uintptr_t)win_impl;
	#		else
				return (void*)(uintptr_t)glfwGetX11Window(_window);
	#		endif
	#	elif BX_PLATFORM_OSX
			return glfwGetCocoaWindow(_window);
	#	elif BX_PLATFORM_WINDOWS
			return glfwGetWin32Window(_window);
	#	endif // BX_PLATFORM_

		throw std::runtime_error("[RawrBOX-Render] Failed to detect platform");
	}

	static void* getNativeDisplayHandle() {
	#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	#		if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
				return glfwGetWaylandDisplay();
	#		else
				return glfwGetX11Display();
	#		endif
	#	else
			return NULL;
	#	endif // BX_PLATFORM_*
	}

	void Renderer::initialize(bgfx::ViewId id, int width, int height, uint32_t flags) {
		this->_kClearView = id;

		glfwSetErrorCallback(glfw_errorCallback);
		if (!glfwInit()) throw std::runtime_error("[RawrBOX-Render] Failed to initialize glfw");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable opengl
		this->_handle = glfwCreateWindow(width, height, this->_title.c_str(), nullptr, nullptr);
		if (this->_handle == nullptr) throw std::runtime_error(fmt::format("[RawrBOX-Render] Failed to initialize window [{} - {}x{}]", this->_title, width, height));

		if((flags & RenderFlags::Features::MULTI_THREADED) == 0) bgfx::renderFrame();

		bgfx::Init init;
		init.type = this->_renderType;
		init.resolution.width = static_cast<uint32_t>(width);
		init.resolution.height = static_cast<uint32_t>(height);

		auto resetFlags = BGFX_RESET_NONE;
		if((flags & RenderFlags::Features::VSYNC) > 0) resetFlags |= BGFX_RESET_VSYNC;
		if((flags & RenderFlags::Features::ANTI_ALIAS) > 0) resetFlags |= BGFX_RESET_MAXANISOTROPY;

		init.resolution.reset = resetFlags;
		init.platformData.nwh = glfwNativeWindowHandle(GLFWHANDLE);
		init.platformData.ndt = getNativeDisplayHandle();

		if (!bgfx::init(init)) throw std::runtime_error("[RawrBOX-Render] Failed to initialize bgfx");

		auto debugFlags = BGFX_DEBUG_NONE;
		if((flags & RenderFlags::Debug::WIREFRAME) > 0) debugFlags |= BGFX_DEBUG_WIREFRAME;
		if((flags & RenderFlags::Debug::STATS) > 0) debugFlags |= BGFX_DEBUG_STATS;

		bgfx::setViewClear(this->_kClearView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
		bgfx::setViewRect(this->_kClearView, 0, 0, bgfx::BackbufferRatio::Equal);

		bgfx::setDebug(debugFlags);

		glfwSetWindowUserPointer(GLFWHANDLE, this);
		glfwSetKeyCallback(GLFWHANDLE, callbacks_key);
		glfwSetCharCallback(GLFWHANDLE, callbacks_char);
		glfwSetScrollCallback(GLFWHANDLE, callbacks_scroll);
		glfwSetWindowSizeCallback(GLFWHANDLE, callbacks_resize);
		glfwSetWindowFocusCallback(GLFWHANDLE, callbacks_focus);
		glfwSetCursorPosCallback(GLFWHANDLE, callbacks_mouseMove);
		glfwSetMouseButtonCallback(GLFWHANDLE, callbacks_mouseKey);
		glfwSetWindowCloseCallback(GLFWHANDLE, callbacks_windowClose);
	}

	void Renderer::setMonitor(int monitor) {
		this->_monitor = monitor;
	}

	void Renderer::setRenderer(bgfx::RendererType::Enum render) {
		if(!this->isRendererSupported(render)) throw std::runtime_error(fmt::format("[RawrBOX-Render] Renderer {} is not supported by your OS", bgfx::getRendererName(render)));
		this->_renderType = render;
	}

	void Renderer::setClearColor(uint32_t clearColor) {
		this->_clearColor = clearColor;
	}

	void Renderer::setTitle(const std::string& title) {
		this->_title = title;
	}

	void Renderer::swapBuffer() const {
		if (this->_handle == nullptr) return;
		bgfx::touch(this->_kClearView); // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
	}

	void Renderer::render() const {
		if (this->_handle == nullptr) return;
		bgfx::frame();
	}

	void Renderer::shutdown() {
		bgfx::shutdown();
		glfwTerminate();
	}

	void Renderer::pollEvents() {
		if (this->_handle == nullptr) return;
		glfwPollEvents();
	}

	#pragma region UTILS
	void Renderer::close() {
		if (this->_handle == nullptr) return;
		glfwDestroyWindow(GLFWHANDLE);
		this->_handle = nullptr;
	}

	bool Renderer::isRendererSupported(bgfx::RendererType::Enum render) {
		bgfx::RendererType::Enum supportedRenderers[bgfx::RendererType::Count];
		uint8_t num = bgfx::getSupportedRenderers(BX_COUNTOF(supportedRenderers), supportedRenderers);
		for (uint8_t i = 0; i < num; ++i) {
			if(supportedRenderers[i] == render) return true;
		}

		return false;
	}

	Vector2i Renderer::getSize() const {
		if (this->_handle == nullptr) throw std::runtime_error("[RawrBOX-Render] Renderer not initialized, handle not found");
		Vector2i ret;
		glfwGetWindowSize(GLFWHANDLE, &ret.x, &ret.y);

		return ret;
	}

	Vector2i Renderer::getMousePos() const {
		if (this->_handle == nullptr) throw std::runtime_error("[RawrBOX-Render] Renderer not initialized, handle not found");
		double x, y;

		glfwGetCursorPos(GLFWHANDLE, &x, &y);
		return Vector2i(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)));
	}

	bool Renderer::getShouldClose() const {
		if (this->_handle == nullptr) return false;
		return glfwWindowShouldClose(GLFWHANDLE);
	}

	void Renderer::setShouldClose(bool close) const {
		if (this->_handle == nullptr) return;
		glfwSetWindowShouldClose(GLFWHANDLE, close ? 1 : 0);
	}
	#pragma endregion

	#pragma region EVENTS
	void Renderer::callbacks_windowClose(GLFWwindow* whandle) {
		auto& renderer = glfwHandleToRenderer(whandle);
		renderer.onWindowClose(renderer);
	}

	void Renderer::callbacks_resize(GLFWwindow* whandle, int width, int height) {
		auto& renderer = glfwHandleToRenderer(whandle);

		bgfx::reset(static_cast<uint32_t>(width), static_cast<uint32_t>(height), BGFX_RESET_VSYNC);
		bgfx::setViewRect(renderer._kClearView, 0, 0, bgfx::BackbufferRatio::Equal);

		renderer.onResize(renderer, {width, height});
	}

	void Renderer::callbacks_key(GLFWwindow* whandle, int key, int scancode, int action, int mods) {
		if (key < 0) return;

		auto& renderer = glfwHandleToRenderer(whandle);
		renderer.onKey(renderer,
						static_cast<unsigned int>(key),
						static_cast<unsigned int>(scancode),
						static_cast<unsigned int>(action),
						static_cast<unsigned int>(mods)
		);

		if (action == GLFW_REPEAT) return;
		renderer.keysIn[key] = action != GLFW_RELEASE ? 1 : 0;
	}

	void Renderer::callbacks_mouseKey(GLFWwindow* whandle, int button, int action, int mods) {
		if (button < 0) return;

		auto& renderer = glfwHandleToRenderer(whandle);
		auto pos = renderer.getMousePos();

		renderer.onMouseKey(renderer,
							pos,
							static_cast<unsigned int>(button),
							static_cast<unsigned int>(action),
							static_cast<unsigned int>(mods)
		);

		renderer.mouseIn[button] = action == GLFW_PRESS ? 1 : 0;
	}

	void Renderer::callbacks_char(GLFWwindow* whandle, unsigned int ch) {
		auto& renderer = glfwHandleToRenderer(whandle);
		renderer.onChar(renderer, ch);
	}

	void Renderer::callbacks_scroll(GLFWwindow* whandle, double x, double y) {
		auto& renderer = glfwHandleToRenderer(whandle);

		auto size = renderer.getSize();
		auto pos = renderer.getMousePos();

		if (pos.x < 0 || pos.y < 0 || pos.x > size.x || pos.y > size.y) return; // Outside window
		renderer.onScroll(renderer, pos, {static_cast<int>(x * 10), static_cast<int>(y * 10)});
	}

	void Renderer::callbacks_mouseMove(GLFWwindow* whandle, double x, double y) {
		auto& renderer = glfwHandleToRenderer(whandle);
		renderer.onMouseMove(renderer, {static_cast<int>(x), static_cast<int>(y)});
	}

	void Renderer::callbacks_focus(GLFWwindow* whandle, int focus) {
		auto& renderer = glfwHandleToRenderer(whandle);

		renderer.hasFocus = focus == 1;
		renderer.onFocus(renderer, focus);
	}
	#pragma endregion

	Renderer::~Renderer() { this->close(); }
}
