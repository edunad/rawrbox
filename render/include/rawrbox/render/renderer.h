#pragma once

#include <rawrbox/utils/event.hpp>
#include <rawrbox/math/vector2.hpp>

#include <bgfx/platform.h>

#include <string>
#include <stdint.h>
#include <unordered_map>

struct GLFWwindow;
namespace rawrBox {
#pragma region FLAGS
	namespace RenderFlags {
		const uint32_t NONE = 0;
		const uint32_t RESIZABLE = 1 << 1;

		namespace Window {
			const uint32_t FULLSCREEN = 1 << 2;
			const uint32_t WINDOWED = 1 << 3;
			const uint32_t WINDOWED_FULLSCREEN = 1 << 4;
		};

		namespace Features {
			const uint32_t VSYNC = 1 << 5;
			const uint32_t ANTI_ALIAS = 1 << 6;
			const uint32_t MULTI_THREADED = 1 << 7;
		};

		namespace Debug {
			const uint32_t WIREFRAME = 1 << 8;
			const uint32_t STATS = 1 << 9;
		};
	};
#pragma endregion

	class Renderer;
#pragma region EVENTS
	using OnFocusCallback = Event<Renderer&, bool>;
	using OnCharCallback = Event<Renderer&, unsigned int>;
	using OnResizeCallback = Event<Renderer&, const Vector2i&>;
	using OnScrollCallback = Event<Renderer&, const Vector2i&, const Vector2i&>;
	using OnMouseMoveCallback = Event<Renderer&, const Vector2i&>;
	using OnKeyCallback = Event<Renderer&, unsigned int, unsigned int, unsigned int, unsigned int>;
	using OnMouseKeyCallback = Event<Renderer&, const Vector2i&, unsigned int, unsigned int, unsigned int>;
	using OnWindowClose = Event<Renderer&>;
#pragma endregion

	class Renderer {
		private:
			void* _handle = nullptr;

			// Default settings
			std::string _title = "RawrBOX - Window";

			bgfx::RendererType::Enum _renderType = bgfx::RendererType::Count;
			bgfx::ViewId _kClearView = 0;

			uint32_t _clearColor = 0x000000FF;
			int _monitor = -1;
			// -----

			#pragma region CALLBACKS
			static void callbacks_focus(GLFWwindow* whandle, int focus);
			static void callbacks_char(GLFWwindow* whandle, unsigned int ch);
			static void callbacks_scroll(GLFWwindow* whandle, double x, double y);
			static void callbacks_mouseMove(GLFWwindow* whandle, double x, double y);
			static void callbacks_resize(GLFWwindow* whandle, int width, int height);
			static void callbacks_mouseKey(GLFWwindow* whandle, int button, int action, int mods);
			static void callbacks_key(GLFWwindow* whandle, int key, int scancode, int action, int mods);
			static void callbacks_windowClose(GLFWwindow* whandle);
			#pragma endregion
		public:

			std::unordered_map<unsigned int, unsigned char> keysIn;
			std::unordered_map<unsigned int, unsigned char> mouseIn;
			bool hasFocus = false;

			#pragma region CALLBACKS
			OnKeyCallback onKey;
			OnCharCallback onChar;
			OnFocusCallback onFocus;
			OnResizeCallback onResize;
			OnScrollCallback onScroll;
			OnMouseKeyCallback onMouseKey;
			OnMouseMoveCallback onMouseMove;
			OnWindowClose onWindowClose;
			#pragma endregion

			void initialize(bgfx::ViewId id, int width, int height, uint32_t flags = RenderFlags::NONE);

			void setMonitor(int monitor);
			void setRenderer(bgfx::RendererType::Enum render);
			void setClearColor(uint32_t clearColor);
			void setTitle(const std::string& title);

			#pragma region RENDERING
			void swapBuffer() const;
			void render() const;
			void shutdown();
			void pollEvents();
			#pragma endregion

			#pragma region UTILS
			void close();
			bool getShouldClose() const;
			void setShouldClose(bool close) const;

			bool isRendererSupported(bgfx::RendererType::Enum render);

			Vector2i getSize() const;
			Vector2i getMousePos() const;
			#pragma endregion

			~Renderer();
	};
}
