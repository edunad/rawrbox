
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/frame.hpp>

namespace rawrbox {
	UIFrame::~UIFrame() {
		this->_stripes = nullptr;
		this->_overlay = nullptr;
		this->_closeButton.reset();

		this->_consola = nullptr;
	}

	void UIFrame::initialize() {
		this->_stripes = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("content/textures/ui/stripe.png")->get();
		this->_overlay = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("content/textures/ui/overlay/overlay.png")->get();

		this->_consola = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("consola.ttf")->getSize(11);

		// Build close button ---
		auto size = this->getSize();

		this->_closeButton = this->createChild<rawrbox::UIButton>();
		this->_closeButton->setTexture("content/textures/ui/icons/close.png");
		this->_closeButton->setSize({30, this->_titleSize - 1});
		this->_closeButton->setPos({size.x - 30, -this->_titleSize});
		this->_closeButton->setTextureSize({8, 8});
		this->_closeButton->setTextureColor(Colors::Black);
		this->_closeButton->setEnabled(true);
		this->_closeButton->setBorder(false);
		this->_closeButton->setBackgroundColor(Colors::Transparent);
		this->_closeButton->setVisible(this->_closable);
		this->_closeButton->initialize();

		this->_closeButton->onClick += [this]() {
			auto ref = this->getRef<rawrbox::UIFrame>();

			ref->onClose();
			ref->remove();
		};
	}

	const rawrbox::Vector2f UIFrame::getDrawOffset() const {
		return {0, 18.F}; // Title offset
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
		rawrbox::UIBase::setSize(size);
		if (this->_closeButton != nullptr) this->_closeButton->setPos({size.x - 30, -this->_titleSize});
	}
	// -------

	// INPUTS ---
	void UIFrame::mouseMove(const rawrbox::Vector2i& mousePos) {
		if (!this->_dragging) return;

		auto p = this->getParent<>();
		this->setPos((this->getPos() + (mousePos.cast<float>() - this->_dragStart)).clamp(p->getPos(), p->getPos() + p->getSize() - this->getSize()));
	}

	void UIFrame::mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) {
		if (!this->_draggable || mousePos.y >= 18) return; // or Touching title

		this->_dragging = true;
		this->_dragStart = mousePos.cast<float>() + this->getDrawOffset();
	}

	void UIFrame::mouseUp(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) {
		this->_dragging = false;
	}
	// -----

	// DRAWING ---
	void UIFrame::draw(rawrbox::Stencil& stencil) {
		auto& size = getSize();

		// Panel Background
		stencil.drawBox({}, size, Color::RGBHex(0x0C0C0C));

		// Title
		stencil.drawBox({}, {size.x, this->_titleSize}, this->_titleColor);
		if (this->_consola != nullptr) {
			stencil.drawText(*this->_consola, this->_title, {5, 8}, Color::RGBAHex(0x000000BA), rawrbox::Alignment::Left, rawrbox::Alignment::Center);
		}

		if (this->_closable) {
			stencil.drawTexture({size.x - 36, 0}, {6, this->_titleSize}, *this->_stripes, Color::RGBAHex(0x0000004A), {}, {1, static_cast<float>(this->_titleSize) / static_cast<float>(this->_stripes->getSize().y / 2)});
		}

		// Title bottom border
		stencil.drawLine({0, this->_titleSize}, {size.x, this->_titleSize}, Color::RGBAHex(0x0000004A));
	}

	void UIFrame::afterDraw(Stencil& stencil) {
		if (this->_overlay == nullptr) return;

		auto& size = this->getSize();
		stencil.drawTexture({}, size, *this->_overlay, Color::RGBAHex(0xFFFFFF0A), {}, {static_cast<float>(size.x) / static_cast<float>(this->_overlay->getSize().x / 2), static_cast<float>(size.y) / static_cast<float>(this->_overlay->getSize().y / 2)});
	}
	// -----

} // namespace rawrbox
