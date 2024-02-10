#pragma once

#include <rawrbox/render/window.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

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

		virtual void setSize(const rawrbox::Vector2& size) const;
		[[nodiscard]] virtual rawrbox::Vector2 getSize() const;

		virtual void setPos(const rawrbox::Vector2& pos) const;
		[[nodiscard]] virtual rawrbox::Vector2 getPos() const;

		[[nodiscard]] virtual float getAspectRatio() const;

		[[nodiscard]] virtual rawrbox::Vector2 getMousePos() const;

		[[nodiscard]] virtual bool isKeyDown(int key) const;
		[[nodiscard]] virtual bool isMouseDown(int key) const;
		// -------

		virtual void close();

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
