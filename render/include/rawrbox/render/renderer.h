#pragma once

#include <rawrbox/utils/event.hpp>

#include <bgfx/platform.h>

#include <string>
#include <stdint.h>

#include <unordered_map>

struct GLFWwindow;
namespace rawrbox::render {

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
		};

		namespace Debug {
			const uint32_t WIREFRAME = 1 << 7;
			const uint32_t STATS = 1 << 8;
		};
	};

#pragma region EVENTS
	using OnFocusCallback = rawrbox::utils::Event<Window&, bool>;
	using OnCharCallback = rawrbox::utils::Event<Window&, unsigned int>;
	using OnResizeCallback = rawrbox::utils::Event<Window&, const math::Vector2i&>;
	using OnScrollCallback = rawrbox::utils::Event<Window&, const math::Vector2i&, const math::Vector2i&>;
	using OnMouseMoveCallback = rawrbox::utils::Event<Window&, const math::Vector2i&>;
	using OnKeyCallback = rawrbox::utils::Event<Window&, unsigned int, unsigned int, unsigned int, unsigned int>;
	using OnMouseKeyCallback = rawrbox::utils::Event<Window&, const math::Vector2i&, unsigned int, unsigned int, unsigned int>;
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

			OnKeyCallback onKey;
			OnCharCallback onChar;
			OnFocusCallback onFocus;
			OnResizeCallback onResize;
			OnScrollCallback onScroll;
			OnMouseKeyCallback onMouseKey;
			OnMouseMoveCallback onMouseMove;
			#pragma endregion
		public:

			std::unordered_map<unsigned int, unsigned char> keysIn;
			std::unordered_map<unsigned int, unsigned char> mouseIn;
			bool hasFocus = false;

			void initialize(int width, int height, uint32_t flags = RenderFlags::NONE);

			void setMonitor(int monitor);
			void setRenderer(bgfx::RendererType::Enum render);
			void setClearColor(uint32_t clearColor);
			void setTitle(const std::string& title);

			#pragma region RENDERING
			void swapBuffer() const;
			void shutdown();
			void pollEvents();
			#pragma endregion

			#pragma region UTILS
			bool getShouldClose() const;
			void setShouldClose(bool close) const;

			bool isRendererSupported(bgfx::RendererType::Enum render);
			math::Vector2i getSize() const;
			math::Vector2i getMousePos() const;
			#pragma endregion
	};
}
