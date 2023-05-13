
#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/static.hpp>

namespace rawrbox {
	UIFrame::UIFrame() {
		/*auto& content = IasGame::getInstance().content;
		consolas = content.getFile<ContentEntryFont>("content/fonts/consola.ttf")->getSize(11);

		stripes = content.getFile<ContentEntryTexture>("content/textures/ui/components/stripe.png")->texture;
		overlay = content.getFile<ContentEntryTexture>("content/textures/ui/components/overlay.png")->texture;*/
	}

	void UIFrame::initialize() {
		rawrbox::ROOT_UI->setFocus(this->getRef<UIFrame>());
	}

	// UTILS -----
	const rawrbox::Color& UIFrame::getTitleColor() const { return this->_titleColor; }
	void UIFrame::setTitleColor(const rawrbox::Color& color) { this->_titleColor = color; }

	void UIFrame::setTitle(const std::string& title) { this->_title = title; }
	const std::string& UIFrame::getTitle() const { return this->_title; }

	void UIFrame::setClosable(bool closable) {
		this->_closable = closable;
		// closable ? closeButton->show() : closeButton->hide();
	}
	bool UIFrame::isClosable() const { return this->_closable; }

	void UIFrame::setDraggable(bool draggable) { this->_draggable = draggable; }
	bool UIFrame::isDraggable() const { return this->_draggable; }
	// -------

	// INPUTS ---
	void UIFrame::mouseMove(const rawrbox::Vector2i& mousePos) {
		if (!this->_dragging) return;

		auto p = this->getParent();
		this->setPos((getPos() + (mousePos.cast<float>() - this->_dragStart)).clamp(p->getPos(), p->getPos() + p->getSize() - getSize()));
	}

	void UIFrame::mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) {
		if (!this->_draggable || mousePos.y >= 18) return; // or Touching title

		this->_dragging = true;
		this->_dragStart = mousePos.cast<float>();
	}

	void UIFrame::mouseUp(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) {
		this->_dragging = false;
	}
	// -----

	// DRAWING ---
	void UIFrame::draw(Stencil& stencil) {
		auto& size = getSize();
		const float titleSize = 18.F;

		// Panel Background
		stencil.drawBox({}, size, Color::RGBHex(0x0C0C0C));

		// Title
		stencil.drawBox({}, {size.x, titleSize}, this->_titleColor);
		// stencil.drawText(*consolas, this->_title, {5, 8}, Color::RGBAHex(0x000000BA), Stencil::TextAlignment::Left, Stencil::TextAlignment::Center);

		/*auto& size = getSize();
		const auto titleSize = 18;

		// Panel Background
		stencil.drawBox({}, size, Color::RGBHex(0x0C0C0C));

		// Title
		stencil.drawBox({}, {size.x, titleSize}, this->titleColor);
		stencil.drawText(*consolas, fmt::format(":// {}", title), {5, 8}, Color::RGBAHex(0x000000BA), Stencil::TextAlignment::Left, Stencil::TextAlignment::Center);

		if (closable) {
			stencil.drawTexture({size.x - 36, 0}, {6, titleSize}, *stripes, Color::RGBAHex(0x0000004A), {}, {1, static_cast<float>(titleSize) / static_cast<float>(stripes->getSize().y / 2)});
		}

		// Title bottom border
		stencil.drawBox({0, titleSize - 2}, {size.x, 2}, Color::RGBAHex(0x0000004A));

		// Bottom frame border
		stencil.drawBoxOutlined({0, titleSize}, {size.x, size.y - titleSize}, 2, Color::RGBAHex(0x0000005A));*/
	}

	void UIFrame::afterDraw(Stencil& stencil) {
		/*auto& size = getSize();

		stencil.drawTexture({}, size, *overlay, Color::RGBAHex(0xFFFFFF0A), {}, {static_cast<float>(size.x) / static_cast<float>(overlay->getSize().x / 2), static_cast<float>(size.y) / static_cast<float>(overlay->getSize().y / 2)});*/
	}
	// -----

} // namespace rawrbox
