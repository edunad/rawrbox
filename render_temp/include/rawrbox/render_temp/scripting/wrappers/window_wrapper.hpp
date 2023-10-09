#pragma once

#include <rawrbox/render_temp/window.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class WindowWrapper {
	protected:
		rawrbox::Window* _window = nullptr;

	public:
		WindowWrapper(rawrbox::Window* window);
		WindowWrapper(const WindowWrapper&) = default;
		WindowWrapper(WindowWrapper&&) = default;
		WindowWrapper& operator=(const WindowWrapper&) = default;
		WindowWrapper& operator=(WindowWrapper&&) = default;
		virtual ~WindowWrapper() = default;

		// Utils ----
		virtual void setTitle(const std::string& title);

		virtual void hideCursor(bool hidden);
		virtual void setCursor(uint32_t cursor);

		virtual void close();

		[[nodiscard]] virtual rawrbox::Vector2i getSize() const;
		[[nodiscard]] virtual float getAspectRatio() const;

		[[nodiscard]] virtual rawrbox::Vector2i getMousePos() const;

		[[nodiscard]] virtual bool isKeyDown(int key) const;
		[[nodiscard]] virtual bool isMouseDown(int key) const;
		// -------

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
