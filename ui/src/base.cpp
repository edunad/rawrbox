
#include <rawrbox/ui/base.hpp>

namespace rawrbox {
	UIBase::UIBase(bool alwaysOnTop) : _alwaysOnTop(alwaysOnTop) {}

	// UTILS ---
	const bool UIBase::alwaysOnTop() const { return this->_alwaysOnTop; }

	void UIBase::setVisible(bool visible) { this->_visible = visible; }
	const bool UIBase::visible() const { return this->_visible; }

	void UIBase::setFocused(bool focused) { this->_focused = focused; }
	const bool UIBase::focused() const { return this->_focused; }

	void UIBase::setHovering(bool hovering) { this->_hovering = hovering; }
	const bool UIBase::hovering() const { return this->_hovering; }

	const rawrbox::Vector2f UIBase::getPosAbsolute() const {
		rawrbox::Vector2f ret;

		std::shared_ptr<UIBase> parent = this->getRef<UIBase>();
		while ((parent = parent->getParent<UIBase>()) != nullptr) {
			ret += parent->getPos();
		}

		return ret + getPos();
	}

	void UIBase::bringToFront() {
		auto& children = this->getParent<rawrbox::UIContainer>()->getChildren();
		auto pivot = std::find_if(children.begin(), children.end(), [this](std::shared_ptr<rawrbox::UIBase> el) -> bool {
			return el.get() == this;
		});

		if (pivot == children.end()) return; // Already in front
		// TODO: CHECK IF TOP ONE IS ALWAYS TOP
		std::rotate(pivot, pivot + 1, children.end());
	}
	// --------

	// DRAWING ------
	void UIBase::beforeDraw(rawrbox::Stencil& stencil) {}
	void UIBase::afterDraw(rawrbox::Stencil& stencil) {}
	// --

	// INPUTS ----
	void UIBase::mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) {}
	void UIBase::mouseUp(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) {}
	void UIBase::mouseScroll(const rawrbox::Vector2i& mousePos, const rawrbox::Vector2i& offset) {}
	void UIBase::mouseMove(const rawrbox::Vector2i& mousePos) {}
	void UIBase::key(uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods) {}
	void UIBase::keyChar(uint32_t key) {}
	// ---

	// FOCUS HANDLE ---
	[[nodiscard]] bool UIBase::hitTest(const rawrbox::Vector2f& point) const { return this->_aabb.contains(point); }
	// -----

} // namespace rawrbox
