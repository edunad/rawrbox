#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/event.hpp>

#include <string>

namespace rawrbox {
	class TextureBase;
	class UIButton;

	class UIFrame : public rawrbox::UIContainer {
	protected:
		std::string _title;
		rawrbox::Color _titleColor = rawrbox::Colors::White();

		bool _draggable = true;
		bool _closable = true;

		rawrbox::UIButton* _closeButton = nullptr;

		// DRAGGING --
		bool _dragging = false;
		rawrbox::Vector2f _dragStart = {};
		// ----

		// RESOURCES ---
		rawrbox::TextureBase* _stripes = nullptr;
		rawrbox::TextureBase* _overlay = nullptr;
		// -----------------

	public:
		constexpr static float TITLE_SIZE = 18.F;
		rawrbox::Event<> onClose;

		UIFrame(rawrbox::UIRoot* root);
		UIFrame(const UIFrame&) = default;
		UIFrame(UIFrame&&) = delete;
		UIFrame& operator=(const UIFrame&) = default;
		UIFrame& operator=(UIFrame&&) = delete;
		~UIFrame() override = default;

		[[nodiscard]] rawrbox::Vector2f getDrawOffset() const override;

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
