#pragma once

#include <rawrbox/ui/base.hpp>

#include <array>

namespace rawrbox {
	class UIFrame : public rawrbox::UIBase {
	protected:
		std::string _title;
		rawrbox::Color _titleColor;

		bool _draggable = false;
		bool _closable = false;

		// DRAGGING --
		bool _dragging = false;
		rawrbox::Vector2f _dragStart = {};
		//

	public:
		rawrbox::Event<> onClose;

		UIFrame();

		void initialize();

		// UTILS -----
		[[nodiscard]] const rawrbox::Color& getTitleColor() const;
		void setTitleColor(const rawrbox::Color& color);

		void setTitle(const std::string& title);
		[[nodiscard]] const std::string& getTitle() const;

		void setClosable(bool closable);
		[[nodiscard]] bool isClosable() const;

		void setDraggable(bool draggable);
		[[nodiscard]] bool isDraggable() const;
		// -------

		// INPUTS ---
		void mouseMove(const rawrbox::Vector2i& mousePos) override;
		void mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) override;
		void mouseUp(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) override;
		// -----

		// DRAWING ---
		void draw(rawrbox::Stencil& stencil) override;
		void afterDraw(rawrbox::Stencil& stencil) override;
		// -----
	};
} // namespace rawrbox
