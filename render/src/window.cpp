
#include "rawrbox/utils/path.hpp"
#ifndef ENGINE_DLL
	#define ENGINE_DLL 1
#endif

#ifndef D3D11_SUPPORTED
	#define D3D11_SUPPORTED 0
#endif

#ifndef D3D12_SUPPORTED
	#define D3D12_SUPPORTED 0
#endif

#ifndef GL_SUPPORTED
	#define GL_SUPPORTED 0
#endif

#ifndef VULKAN_SUPPORTED
	#define VULKAN_SUPPORTED 0
#endif

#ifndef METAL_SUPPORTED
	#define METAL_SUPPORTED 0
#endif

#if D3D11_SUPPORTED
	#include <Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h>
#endif

#if D3D12_SUPPORTED
	#include <Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h>
#endif

#if GL_SUPPORTED
	#include <Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>
#endif

#if VULKAN_SUPPORTED
	#include <Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>
#endif

#if METAL_SUPPORTED
	#include <Graphics/GraphicsEngineMetal/interface/EngineFactoryMtl.h>
#endif

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

#include <GLFW/glfw3native.h>

// #include <rawrbox/render_temp/texture/webp.hpp>
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/utils/threading.hpp>

#include <fmt/printf.h>

#include <cmath>
#include <filesystem>
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
		fmt::print("[RawrBox-Window] GLFW error {}: {}\n", error, description);
	}
	// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)

	Window::~Window() { this->close(); }
	Window::Window() {
		int count = 0;
		auto monitors = glfwGetMonitors(&count);

		for (int i = 0; i < count; i++) {
			const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
			this->_screenSizes[i] = {mode->width, mode->height};
		}
	}

	void Window::playIntro() {
		this->_renderer->setOverlayRender(this->_overlay);
		this->_renderer->setWorldRender(this->_world);

		this->onIntroCompleted();

		/*if (this->_skipIntros) {
			this->_renderer->setOverlayRender(this->_overlay);
			this->_renderer->setWorldRender(this->_world);

			this->_introList.clear();
			this->_currentIntro = nullptr;
			this->_introComplete = true;

			this->onIntroCompleted();
			return;
		}

		// Load webp intros -----------------------
		rawrbox::ASYNC::run([this]() {
			// Load ----
			for (auto& intro : this->_introList) {
				intro.second.texture = std::make_shared<rawrbox::TextureWEBP>(intro.first);
				intro.second.texture->setLoop(false);
				intro.second.texture->setSpeed(intro.second.speed);
				intro.second.texture->onEnd += [this]() {
					this->_introComplete = true;
				};

				intro.second.texture->upload();
			}

			// First intro on the list
			this->_currentIntro = &this->_introList.begin()->second;
		});
		// -------------------------

		this->_renderer->setWorldRender([]() {});
		this->_renderer->setOverlayRender([this]() {
			bgfx::dbgTextClear(); // Prevent debug text being rendered on top

			this->_stencil->drawBox({}, this->_size.cast<float>(), rawrbox::Colors::Black());

			if (this->_currentIntro != nullptr) {
				auto screenSize = this->_size.cast<float>();

				if (this->_currentIntro->cover) {
					this->_stencil->drawTexture({0, 0}, {screenSize.x, screenSize.y}, *this->_currentIntro->texture);
				} else {
					auto size = this->_currentIntro->texture->getSize().cast<float>();
					this->_stencil->drawTexture({screenSize.x / 2.F - size.x / 2.F, screenSize.y / 2.F - size.y / 2.F}, {size.x, size.y}, *this->_currentIntro->texture);
				}
			}

			this->_stencil->render();
		});*/
	}

	void Window::create(int width, int height, uint32_t flags) {
		if (rawrbox::RENDER_THREAD_ID == std::this_thread::get_id()) throw std::runtime_error("[RawrBox-Window] 'create' should be called inside engine's 'setupGLFW'!");

		int APIHint = GLFW_NO_API;
#if !PLATFORM_WIN32
		if (this->getRenderDeviceType() == Diligent::RENDER_DEVICE_TYPE_GL) {
			// On platforms other than Windows Diligent Engine
			// attaches to existing OpenGL context
			APIHint = GLFW_OPENGL_API;
		}
#endif

		glfwSetErrorCallback(glfw_errorCallback);
		if (!glfwInit()) throw std::runtime_error("[RawrBox-Window] Failed to initialize glfw");

		glfwWindowHint(GLFW_CLIENT_API, APIHint); // Disable opengl
		if (APIHint == GLFW_OPENGL_API) {
			// We need compute shaders, so request OpenGL 4.2 at least
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		}
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
		if (fullscreen && (windowed || borderless)) throw std::runtime_error("[RawrBox-Window] Only one window flag attribute can be selected");

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

		glfwSetKeyCallback(GLFWHANDLE, callbacks_key);
		glfwSetCharCallback(GLFWHANDLE, callbacks_char);
		glfwSetScrollCallback(GLFWHANDLE, callbacks_scroll);
		glfwSetWindowSizeCallback(GLFWHANDLE, callbacks_resize);
		glfwSetWindowFocusCallback(GLFWHANDLE, callbacks_focus);
		glfwSetCursorPosCallback(GLFWHANDLE, callbacks_mouseMove);
		glfwSetMouseButtonCallback(GLFWHANDLE, callbacks_mouseKey);
		glfwSetWindowCloseCallback(GLFWHANDLE, callbacks_windowClose);

		// Initialize renderer
		this->_windowFlags = flags;
		this->_size = {width, height};
		// -------------------

		rawrbox::__OPEN_WINDOWS__++;
	}

	Diligent::RENDER_DEVICE_TYPE Window::getRenderDeviceType() {
		if (this->_renderType != Diligent::RENDER_DEVICE_TYPE_COUNT) return this->_renderType;
#if PLATFORM_LINUX
	#if VULKAN_SUPPORTED
		this->_renderType = Diligent::RENDER_DEVICE_TYPE_VULKAN;
	#else
		this->_renderType = Diligent::RENDER_DEVICE_TYPE_GL;
	#endif
#else
	#if D3D12_SUPPORTED
		this->_renderType = Diligent::RENDER_DEVICE_TYPE_D3D12;
	#elif D3D11_SUPPORTED
		this->_renderType = Diligent::RENDER_DEVICE_TYPE_D3D11;
	#elif VULKAN_SUPPORTED
		this->_renderType = Diligent::RENDER_DEVICE_TYPE_VULKAN;
	#else
		this->_renderType = Diligent::RENDER_DEVICE_TYPE_GL;
	#endif
#endif

		return this->_renderType;
	}

	// Should be ran on main render thread!
	void Window::initializeEngine() {
		if (rawrbox::RENDER_THREAD_ID != std::this_thread::get_id()) throw std::runtime_error("[RawrBox-Window] 'initializeBGFX' should be called inside 'init'. Aka the main render thread!");
		if (this->_renderer == nullptr) throw std::runtime_error("[RawrBox-Window] missing renderer! Did you call 'setRenderer' ?");

			// Get native window ----
#if PLATFORM_WIN32
		Diligent::Win32NativeWindow Window{glfwGetWin32Window(GLFWHANDLE)};
#endif

#if PLATFORM_LINUX
		Diligent::LinuxNativeWindow Window;
		Window.WindowId = glfwGetX11Window(GLFWHANDLE);
		Window.pDisplay = glfwGetX11Display();
		if (type == Diligent::RENDER_DEVICE_TYPE_GL)
			glfwMakeContextCurrent(GLFWHANDLE);
#endif

#if PLATFORM_MACOS
		Diligent::MacOSNativeWindow Window;
		if (type == Diligent::RENDER_DEVICE_TYPE_GL)
			glfwMakeContextCurrent(GLFWHANDLE);
		else
			Window.pNSView = GetNSWindowView(GLFWHANDLE);
#endif
		// ------------

		Diligent::SwapChainDesc SCDesc;
		switch (this->getRenderDeviceType()) {
#if D3D11_SUPPORTED
			case Diligent::RENDER_DEVICE_TYPE_D3D11:
				{
	#if ENGINE_DLL
					auto* GetEngineFactoryD3D11 = Diligent::LoadGraphicsEngineD3D11(); // Load the dll and import GetEngineFactoryD3D11() function
	#endif
					auto* pFactoryD3D11 = GetEngineFactoryD3D11();
					this->_pEngineFactory = pFactoryD3D11;

					Diligent::EngineD3D11CreateInfo EngineCI;
					pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &this->_renderer->device, &this->_renderer->context);
					pFactoryD3D11->CreateSwapChainD3D11(this->_renderer->device, this->_renderer->context, SCDesc, Diligent::FullScreenModeDesc{}, Window, &this->_renderer->swapChain);
				}
				break;
#endif

#if D3D12_SUPPORTED
			case Diligent::RENDER_DEVICE_TYPE_D3D12:
				{
	#if ENGINE_DLL
					// Load the dll and import GetEngineFactoryD3D12() function
					auto* GetEngineFactoryD3D12 = Diligent::LoadGraphicsEngineD3D12();
	#endif
					auto* pFactoryD3D12 = GetEngineFactoryD3D12();
					this->_pEngineFactory = pFactoryD3D12;

					Diligent::EngineD3D12CreateInfo EngineCI;
					pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &this->_renderer->device, &this->_renderer->context);
					pFactoryD3D12->CreateSwapChainD3D12(this->_renderer->device, this->_renderer->context, SCDesc, Diligent::FullScreenModeDesc{}, Window, &this->_renderer->swapChain);
				}
				break;
#endif // D3D12_SUPPORTED

#if GL_SUPPORTED
			case Diligent::RENDER_DEVICE_TYPE_GL:
				{
	#if EXPLICITLY_LOAD_ENGINE_GL_DLL
					// Load the dll and import GetEngineFactoryOpenGL() function
					auto GetEngineFactoryOpenGL = Diligent::LoadGraphicsEngineOpenGL();
					auto* pFactoryOpenGL = GetEngineFactoryOpenGL();
	#else
					auto* pFactoryOpenGL = Diligent::GetEngineFactoryOpenGL();
	#endif
					this->_pEngineFactory = pFactoryOpenGL;

					Diligent::EngineGLCreateInfo EngineCI;
					EngineCI.Window = Window;
					pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &this->_renderer->device, &this->_renderer->context, SCDesc, &this->_renderer->swapChain);
				}
				break;
#endif // GL_SUPPORTED

#if VULKAN_SUPPORTED
			case Diligent::RENDER_DEVICE_TYPE_VULKAN:
				{
	#if EXPLICITLY_LOAD_ENGINE_GL_DLL
					// Load the dll and import GetEngineFactoryVk() function
					auto* GetEngineFactoryVk = Diligent::LoadGraphicsEngineVk();
					auto* pFactoryVk = GetEngineFactoryVk();
	#else
					auto* pFactoryVk = Diligent::GetEngineFactoryVk();
	#endif
					this->_pEngineFactory = pFactoryVk;

					Diligent::EngineVkCreateInfo EngineCI;
					pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &this->_renderer->device, &this->_renderer->context);
					pFactoryVk->CreateSwapChainVk(this->_renderer->device, this->_renderer->context, SCDesc, Window, &this->_renderer->swapChain);
				}
				break;
#endif // VULKAN_SUPPORTED
			default: throw std::runtime_error("[RawrBox-Window] Invalid diligent engine");
		}

		if (this->_pEngineFactory == nullptr) throw std::runtime_error("[RawrBox-Window] Failed to initialize engine");
		rawrbox::ENGINE_INITIALIZED = true;

		// Setup shader pipeline
		if (rawrbox::SHADER_FACTORY == nullptr) {
			auto dirs = rawrbox::PathUtils::glob("assets/shaders", true);
			auto paths = fmt::format("{}", fmt::join(dirs, ","));

			this->_pEngineFactory->CreateDefaultShaderSourceStreamFactory(paths.c_str(), &rawrbox::SHADER_FACTORY);
		}
		// -----------

		// Setup renderer
		this->_renderer->init(this->_size);
		// ------------------

		// Setup global util textures ---
		/*if (rawrbox::MISSING_TEXTURE == nullptr) {
			rawrbox::MISSING_TEXTURE = std::make_shared<rawrbox::TextureMissing>();
			rawrbox::MISSING_TEXTURE->upload();
		}

		if (rawrbox::WHITE_TEXTURE == nullptr) {
			rawrbox::WHITE_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Colors::White());
			rawrbox::WHITE_TEXTURE->upload();
		}

		if (rawrbox::BLACK_TEXTURE == nullptr) {
			rawrbox::BLACK_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Colors::Black());
			rawrbox::BLACK_TEXTURE->upload();
		}

		if (rawrbox::NORMAL_TEXTURE == nullptr) {
			rawrbox::NORMAL_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Color::RGBHex(0xbcbcff));
			rawrbox::NORMAL_TEXTURE->upload();
		}*/
		// ------------------

		// Setup stencil ----
		this->_stencil = std::make_unique<rawrbox::Stencil>(this->getSize());
		this->_stencil->upload();
		// ------------------

		// INTRO ----
		this->playIntro();
		// ---------

		// Setup resize events ---
		this->onResize += [this](auto&, auto& size) {
			if (this->_stencil != nullptr) this->_stencil->resize(size);
			this->_renderer->resize(this->_size);
		};
		// ------------------------
	}

	void Window::setMonitor(int monitor) {
		this->_monitor = monitor;
	}

	void Window::setTitle(const std::string& title) {
		this->_title = title;
	}

	void Window::overridePostWorld(std::function<void()> fnc) {
		this->_renderer->overridePostWorld(fnc);
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
		if (pixels.empty()) return;

		std::memcpy(this->_cursorPixels.data(), pixels.data(), pixels.size() * sizeof(uint8_t));
		if (GLFWCURSOR != nullptr) glfwDestroyCursor(GLFWCURSOR); // Delete old one

		GLFWimage image = {};
		image.pixels = this->_cursorPixels.data();
		image.width = 16;
		image.height = 16;

		auto cursor = glfwCreateCursor(&image, 0, 0);
		this->_cursor = cursor;

		glfwSetCursor(GLFWHANDLE, cursor);
	}
	// -------------------

	void Window::shutdown() {
		if (this->_handle == nullptr) return;

		rawrbox::RENDERER = nullptr;
		rawrbox::MAIN_CAMERA = nullptr;

		glfwDestroyWindow(GLFWHANDLE); // Optional
		glfwTerminate();
	}

	void Window::update() {
		/*if (this->_currentIntro != nullptr) {
			if (this->_introComplete) {
				this->_introList.erase(this->_introList.begin());

				// Done?
				if (this->_introList.empty()) {
					this->_renderer->setOverlayRender(this->_overlay);
					this->_renderer->setWorldRender(this->_world);

					this->_currentIntro = nullptr;
					this->onIntroCompleted();
				} else {
					this->_currentIntro = &this->_introList.begin()->second;
					this->_introComplete = false;
				}

				return;
			}

			this->_currentIntro->texture->update();
		} else {
			if (this->_camera != nullptr) {
				this->_camera->update();
			}
		}*/

		if (this->_camera != nullptr) {
			this->_camera->update();
		}
	}

	// INTRO ------
	void Window::skipIntros(bool skip) {
		if (skip) fmt::print("[RawrBox] Skipping intros :(\n");
		this->_skipIntros = skip;
	}

	void Window::addIntro(const std::filesystem::path& webpPath, float speed, bool cover) {
		/*if (webpPath.extension() != ".webp") throw std::runtime_error(fmt::format("[RawrBox-Window] Invalid intro '{}', format needs to be .webp!", webpPath.generic_string()));

		rawrbox::RawrboxIntro intro;
		intro.cover = cover;
		intro.speed = speed;
		intro.texture = nullptr;

		this->_introList[webpPath.generic_string()] = intro;*/
	}
	// ----------------

	void Window::unblockPoll() {
		glfwPostEmptyEvent();
	}

	void Window::pollEvents() {
		if (this->_handle == nullptr) return;
		glfwWaitEvents();
	}

	// DRAW ------
	void Window::render() const {
		if (!rawrbox::ENGINE_INITIALIZED) return;
		this->_renderer->render();
	}

	// -------------------

	// ------UTILS
	void Window::close() {
		/*if (rawrbox::__OPEN_WINDOWS__-- <= 0) {
			rawrbox::MISSING_TEXTURE.reset();
			rawrbox::WHITE_TEXTURE.reset();
			rawrbox::BLACK_TEXTURE.reset();
			rawrbox::NORMAL_TEXTURE.reset();
		}*/

		this->_stencil.reset();
		this->_camera.reset();
		this->_renderer.reset();

		if (GLFWHANDLE != nullptr) glfwDestroyWindow(GLFWHANDLE);
		this->_handle = nullptr;
		if (GLFWCURSOR != nullptr) glfwDestroyCursor(GLFWCURSOR);
		this->_cursor = nullptr;
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

	uint32_t Window::getWindowFlags() const {
		return this->_windowFlags;
	}

	rawrbox::Stencil& Window::getStencil() const {
		return *this->_stencil;
	}

	bool Window::isKeyDown(int key) const {
		if (this->_handle == nullptr) return false;
		return glfwGetKey(GLFWHANDLE, key) == GLFW_PRESS;
	}

	bool Window::isMouseDown(int key) const {
		if (this->_handle == nullptr) return false;
		return glfwGetMouseButton(GLFWHANDLE, key) == GLFW_PRESS;
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
		window.onWindowClose(window);
	}

	void Window::callbacks_resize(GLFWwindow* whandle, int width, int height) {
		rawrbox::runOnRenderThread([whandle, width, height]() {
			auto& window = glfwHandleToRenderer(whandle);
			window._size = {width, height};

			window.onResize(window, {width, height});
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
	// --------------------

} // namespace rawrbox
