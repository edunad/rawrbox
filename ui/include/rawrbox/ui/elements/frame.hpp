#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/ui/base.hpp>
#include <rawrbox/utils/event.hpp>

#include <string>

namespace rawrbox {
	class UIFrame : public rawrbox::UIBase {
	protected:
		std::string _title;
		rawrbox::Color _titleColor = rawrbox::Colors::White;

		bool _draggable = true;
		bool _closable = true;

		// DRAGGING --
		bool _dragging = false;
		rawrbox::Vector2f _dragStart = {};
		//

		// RESOURCES ---
		std::shared_ptr<rawrbox::TextureImage> _stripes = nullptr;
		std::shared_ptr<rawrbox::TextureImage> _overlay = nullptr;
		std::weak_ptr<rawrbox::Font> _consola;
		// -----------------

	public:
		rawrbox::Event<> onClose;

		virtual ~UIFrame() override;
		virtual void initialize();

		// UTILS -----
		[[nodiscard]] const rawrbox::Color& getTitleColor() const;
		virtual void setTitleColor(const rawrbox::Color& color);

		virtual void setTitle(const std::string& title);
		[[nodiscard]] virtual const std::string& getTitle() const;

		virtual void setClosable(bool closable);
		[[nodiscard]] virtual bool isClosable() const;

		virtual void setDraggable(bool draggable);
		[[nodiscard]] virtual bool isDraggable() const;
		// -------

		// INPUTS ---
		virtual void mouseMove(const rawrbox::Vector2i& mousePos) override;
		virtual void mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) override;
		virtual void mouseUp(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) override;
		// -----

		// DRAWING ---
		virtual void draw(rawrbox::Stencil& stencil) override;
		virtual void afterDraw(rawrbox::Stencil& stencil) override;
		// -----
	};
} // namespace rawrbox
