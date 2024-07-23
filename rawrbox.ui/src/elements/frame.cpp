
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/root.hpp>

namespace rawrbox {
	UIFrame::UIFrame(rawrbox::UIRoot* root) : rawrbox::UIContainer(root), _closeButton(this->createChild<rawrbox::UIButton>()) {
		this->_stripes = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/ui/stripe.png")->get();
		this->_overlay = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/ui/overlay/overlay.png")->get();

		// Build close button ---
		auto size = this->getSize();

		this->_closeButton->setTexture("./assets/textures/ui/icons/close.png");
		this->_closeButton->setSize({30, TITLE_SIZE - 1});
		this->_closeButton->setPos({size.x - 30, -TITLE_SIZE});
		this->_closeButton->setTextureSize({8, 8});
		this->_closeButton->setTextureColor(Colors::Black());
		this->_closeButton->setEnabled(true);
		this->_closeButton->setBorder(false);
		this->_closeButton->setBackgroundColor(Colors::Transparent());
		this->_closeButton->setVisible(this->_closable);

		this->_closeButton->onClick += [this]() {
			this->onClose();
			this->remove();
		};
	}

	rawrbox::Vector2f UIFrame::getDrawOffset() const {
		return {0, TITLE_SIZE}; // Title offset
	}

	// UTILS -----
	const rawrbox::Color& UIFrame::getTitleColor() const { return this->_titleColor; }
	void UIFrame::setTitleColor(const rawrbox::Color& color) { this->_titleColor = color; }

	void UIFrame::setTitle(const std::string& title) { this->_title = title; }
	const std::string& UIFrame::getTitle() const { return this->_title; }

	void UIFrame::setClosable(bool closable) {
		this->_closable = closable;
		if (this->_closeButton != nullptr) this->_closeButton->setVisible(closable);
	}
	bool UIFrame::isClosable() const { return this->_closable; }

	void UIFrame::setDraggable(bool draggable) { this->_draggable = draggable; }
	bool UIFrame::isDraggable() const { return this->_draggable; }

	void UIFrame::setSize(const rawrbox::Vector2& size) {
		rawrbox::UIContainer::setSize(size);
		if (this->_closeButton != nullptr) this->_closeButton->setPos({size.x - 30, -TITLE_SIZE});
	}
	// -------

	// INPUTS ---
	void UIFrame::mouseMove(const rawrbox::Vector2i& mousePos) {
		if (!this->_dragging) return;

		if (this->hasParent()) {
			this->setPos((this->getPos() + (mousePos.cast<float>() - this->_dragStart)).clamp(this->_parent->getPos(), this->_parent->getPos() + this->_parent->getSize() - this->getSize()));
		} else {
			const auto& bb = this->getRoot()->getAABB();
			this->setPos((this->getPos() + (mousePos.cast<float>() - this->_dragStart)).clamp(bb.pos, bb.size - this->getSize()));
		}
	}

	void UIFrame::mouseDown(const rawrbox::Vector2i& mousePos, uint32_t /*button*/, uint32_t /*mods*/) {
		if (!this->_draggable || mousePos.y >= 0) return; // or Touching title

		this->_dragging = true;
		this->_dragStart = mousePos.cast<float>() + this->getDrawOffset();
	}

	void UIFrame::mouseUp(const rawrbox::Vector2i& /*mousePos*/, uint32_t /*button*/, uint32_t /*mods*/) {
		this->_dragging = false;
	}
	// -----

	// DRAWING ---
	void UIFrame::draw(rawrbox::Stencil& stencil) {
		const auto& size = this->getSize();

		// Panel Background
		stencil.drawBox({}, size, Color::RGBHex(0x0C0C0C));

		// Title
		stencil.drawBox({}, {size.x, TITLE_SIZE}, this->_titleColor);
		if (rawrbox::DEBUG_FONT_REGULAR != nullptr) {
			stencil.drawText(*rawrbox::DEBUG_FONT_REGULAR, this->_title, {4, 8}, Color::RGBAHex(0x000000D9), rawrbox::Alignment::Left, rawrbox::Alignment::Center);
		}

		if (this->_closable) {
			stencil.drawTexture({size.x - 36, 0}, {6, TITLE_SIZE}, *this->_stripes, Color::RGBAHex(0x0000004A), {}, {1, static_cast<float>(TITLE_SIZE) / static_cast<float>(this->_stripes->getSize().y / 2)});
		}

		// Title bottom border
		stencil.drawLine({0, TITLE_SIZE}, {size.x, TITLE_SIZE}, Color::RGBAHex(0x0000004A));
	}

	void UIFrame::afterDraw(rawrbox::Stencil& stencil) {
		if (this->_overlay == nullptr) return;

		const auto& size = this->getSize();
		auto overlaySize = this->_overlay->getSize().cast<float>() / 2.F;

		stencil.drawTexture({}, size, *this->_overlay, Color::RGBAHex(0xffffff01), {}, {size.x / overlaySize.x, size.y / overlaySize.y});
	}
	// -----

} // namespace rawrbox
