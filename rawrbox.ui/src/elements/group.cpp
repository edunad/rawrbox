#include <rawrbox/render/stencil.hpp>
#include <rawrbox/ui/elements/group.hpp>

namespace rawrbox {
	UIGroup::UIGroup(rawrbox::UIRoot* root) : rawrbox::UIContainer(root) {}

	// UTILS ----
	void UIGroup::setBorder(float border) { this->_border = border; }
	float UIGroup::getBorder() const { return this->_border; }

	void UIGroup::sizeToContents() {
		Vector2 totalSize = {0, 0};

		for (auto& child : getChildren()) {
			const auto& pos = child->getPos();
			const auto& size = child->getSize();

			if (totalSize.x < pos.x + size.x) totalSize.x = pos.x + size.x;
			if (totalSize.y < pos.y + size.y) totalSize.y = pos.y + size.y;
		}

		this->setSize(totalSize);
	}
	// ---------

	// DRAW ----
	void UIGroup::draw(rawrbox::Stencil& stencil) {
		if (this->_border <= 0.F) return;
		const auto& size = this->getContentSize();

		stencil.pushOutline({this->_border, 0});
		stencil.drawBox({this->_border, this->_border}, size - this->_border * 2, rawrbox::Color::RGBAHex(0x0000001A));
		stencil.popOutline();
	}
	// -------
} // namespace rawrbox
