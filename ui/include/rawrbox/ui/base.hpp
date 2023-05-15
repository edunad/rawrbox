#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/ui/container.hpp>

#include <array>

namespace rawrbox {
	class UIBase : public rawrbox::UIContainer {
		bool _alwaysOnTop = false;

	protected:
		bool _hovering = false;
		bool _focused = false;
		bool _visible = true;

	public:
		explicit UIBase(bool alwaysOnTop = false);
		~UIBase() override = default;

		// UTILS -----
		[[nodiscard]] virtual const bool alwaysOnTop() const;

		virtual void setVisible(bool visible);
		[[nodiscard]] virtual const bool visible() const;

		virtual void setFocused(bool visible);
		[[nodiscard]] virtual const bool focused() const;

		virtual void setHovering(bool hovering);
		[[nodiscard]] virtual const bool hovering() const;

		[[nodiscard]] const rawrbox::Vector2f getPosAbsolute() const;
		void bringToFront();
		// ---

		// DRAWING ------
		virtual void beforeDraw(rawrbox::Stencil& stencil);
		virtual void afterDraw(rawrbox::Stencil& stencil);
		// --

		// INPUTS ----
		virtual void mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods);
		virtual void mouseUp(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods);
		virtual void mouseScroll(const rawrbox::Vector2i& mousePos, const rawrbox::Vector2i& offset);
		virtual void mouseMove(const rawrbox::Vector2i& mousePos);
		virtual void key(uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods);
		virtual void keyChar(uint32_t key);
		// ---

		// FOCUS HANDLE ---
		[[nodiscard]] virtual bool hitTest(const rawrbox::Vector2f& point) const;
		// -----
	};
} // namespace rawrbox
