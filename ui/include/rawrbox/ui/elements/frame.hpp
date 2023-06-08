#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/ui/base.hpp>
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/utils/event.hpp>

#include <string>

namespace rawrbox {
	class UIFrame : public rawrbox::UIBase {
	protected:
		float _titleSize = 18.F;

		std::string _title;
		rawrbox::Color _titleColor = rawrbox::Colors::White;

		bool _draggable = true;
		bool _closable = true;

		std::shared_ptr<rawrbox::UIButton> _closeButton = nullptr;

		// DRAGGING --
		bool _dragging = false;
		rawrbox::Vector2f _dragStart = {};
		// ----

		// RESOURCES ---
		rawrbox::TextureImage* _stripes = nullptr;
		rawrbox::TextureImage* _overlay = nullptr;
		rawrbox::Font* _consola = nullptr;
		// -----------------

	public:
		rawrbox::Event<> onClose;

		UIFrame() = default;
		~UIFrame() override;

		void initialize() override;
		[[nodiscard]] const rawrbox::Vector2f getDrawOffset() const override;

		// UTILS -----
		[[nodiscard]] const rawrbox::Color& getTitleColor() const;
		virtual void setTitleColor(const rawrbox::Color& color);

		virtual void setTitle(const std::string& title);
		[[nodiscard]] virtual const std::string& getTitle() const;

		virtual void setClosable(bool closable);
		[[nodiscard]] virtual bool isClosable() const;

		virtual void setDraggable(bool draggable);
		[[nodiscard]] virtual bool isDraggable() const;

		void setSize(const rawrbox::Vector2& size) override;
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
