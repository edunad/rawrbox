#include <rawrbox/render/stencil.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/ui/root.hpp>

namespace rawrbox {
	UIContainer::UIContainer(rawrbox::UIRoot* root) : _root(root) {
		if (_root == nullptr) RAWRBOX_CRITICAL("UI root cannot be null!");
	}

	UIContainer::UIContainer(rawrbox::UIContainer&& other) noexcept : _parent(other._parent), _children(std::move(other._children)), _alwaysOnTop(other._alwaysOnTop), _aabb(other._aabb) {}

	// UTILS ---
	rawrbox::Vector2f UIContainer::getDrawOffset() const { return {}; };

	void UIContainer::setPos(const rawrbox::Vector2f& pos) { this->_aabb.pos = pos; }
	const rawrbox::Vector2f& UIContainer::getPos() const { return this->_aabb.pos; }

	void UIContainer::setSize(const rawrbox::Vector2f& size) { this->_aabb.size = size; }
	const rawrbox::Vector2f& UIContainer::getSize() const { return this->_aabb.size; }
	rawrbox::Vector2f UIContainer::getContentSize() const { return this->getSize() - this->getDrawOffset(); }

	void UIContainer::sizeToParent() {
		if (this->_parent == nullptr) return;
		this->setSize(this->_parent->getContentSize());
	}

	void UIContainer::removeChildren() {
		for (auto& c : this->_children) {
			this->_root->resetHoverFocus(c.get());
			c->removeChildren();
		}

		this->_children.clear();
	}

	void UIContainer::remove() {
		this->removeChildren();

		// Remove self from parent
		if (this->hasParent()) {
			auto& parentChilds = this->_parent->getChildren();
			parentChilds.erase(std::find_if(parentChilds.begin(), parentChilds.end(), [this](auto& c) {
				return c.get() == this;
			}));

			this->_parent = nullptr;
		} else {
			this->_root->removeChild(this);
		}
	}

	void UIContainer::setVisible(bool visible) {
		this->_visible = visible;
		if (!visible) this->_root->resetHoverFocus(this);

		for (auto& c : this->getChildren()) {
			this->_root->resetHoverFocus(c.get()); // Propagate to children so we can unfocus the elements
		}
	}

	bool UIContainer::visible() const {
		return this->_visible;
	}

	void UIContainer::focus() {
		if (this->_root == nullptr) return;
		this->_root->setFocus(this);
	}

	void UIContainer::setFocused(bool focused) { this->_focused = focused; }
	bool UIContainer::focused() const { return this->_focused; }

	void UIContainer::setHovering(bool hovering) { this->_hovering = hovering; }
	bool UIContainer::hovering() const { return this->_hovering; }

	rawrbox::UIRoot* UIContainer::getRoot() const { return this->_root; }

	rawrbox::Vector2f UIContainer::getPosAbsolute() const {
		if (!this->hasParent()) return this->getPos();

		rawrbox::Vector2f ret;
		auto* parent = this->getParent();
		while ((parent = parent->getParent()) != nullptr) {
			ret += parent->getPos();
		}

		return ret + this->getPos();
	}
	// ---

	// PARENTING ---
	std::vector<std::shared_ptr<rawrbox::UIContainer>>& UIContainer::getChildren() { return this->_children; }
	const std::vector<std::shared_ptr<rawrbox::UIContainer>>& UIContainer::getChildren() const { return this->_children; }

	bool UIContainer::hasParent() const { return this->_parent != nullptr; }
	bool UIContainer::hasChildren() const { return !this->_children.empty(); }
	void UIContainer::setParent(rawrbox::UIContainer* elm) { this->_parent = elm; }
	void UIContainer::setRoot(rawrbox::UIRoot* elm) { this->_root = elm; }
	rawrbox::UIContainer* UIContainer::getParent() const { return this->_parent; }
	// --------------

	// SORTING -----

	void UIContainer::setAlwaysTop(bool top) { this->_alwaysOnTop = top; }
	bool UIContainer::alwaysOnTop() const { return this->_alwaysOnTop; }
	void UIContainer::bringToFront() {
		auto& children = this->_root->getChildren();
		auto pivot = std::find_if(children.begin(), children.end(), [this](auto& el) -> bool {
			return el.get() == this;
		});

		if (pivot == children.end()) return; // Already in front

		// TODO: CHECK IF TOP ONE IS ALWAYS TOP
		std::rotate(pivot, pivot + 1, children.end());
	}
	// --------------

	// RENDERING -----
	void UIContainer::internalDraw(rawrbox::UIContainer* elm, rawrbox::Stencil& stencil) {
		if (elm == nullptr || !elm->visible()) return;
		bool canClip = elm->clipOverflow();

		stencil.pushOffset(elm->getPos());
		if (canClip) stencil.pushClipping(rawrbox::Clip{{{0U, 0U}, elm->getSize().cast<uint32_t>()}, false});

		elm->beforeDraw(stencil);
		elm->draw(stencil);

		// Draw children of the element ---
		stencil.pushOffset(elm->getDrawOffset());
		auto& elms = elm->getChildren();
		for (auto& celm : elms) {
			this->internalDraw(celm.get(), stencil);
		}
		stencil.popOffset();
		// -----------

		elm->afterDraw(stencil);

		if (canClip) stencil.popClipping();
		stencil.popOffset();
	}

	void UIContainer::beforeDraw(rawrbox::Stencil& /*_stencil*/) {}
	void UIContainer::afterDraw(rawrbox::Stencil& /*_stencil*/) {}
	void UIContainer::draw(rawrbox::Stencil& /*_stencil*/) {}

	void UIContainer::drawChildren(rawrbox::Stencil& stencil) {
		this->internalDraw(this, stencil);
	}

	bool UIContainer::clipOverflow() const { return !this->_children.empty(); }
	// ----

	// FOCUS HANDLING ------
	bool UIContainer::lockKeyboard() const { return false; }
	bool UIContainer::lockScroll() const { return false; }
	bool UIContainer::lockMouse() const { return false; }

	[[nodiscard]] bool UIContainer::hitTest(const rawrbox::Vector2f& point) const { return this->_aabb.contains(point); }
	// --

	// INPUTS ----
	void UIContainer::mouseDown(const rawrbox::Vector2i& /*_mousePos*/, uint32_t /*_button*/, uint32_t /*_mods*/) {}
	void UIContainer::mouseUp(const rawrbox::Vector2i& /*_mousePos*/, uint32_t /*_button*/, uint32_t /*_mods*/) {}
	void UIContainer::mouseScroll(const rawrbox::Vector2i& /*_mousePos*/, const rawrbox::Vector2i& /*_offset*/) {}
	void UIContainer::mouseMove(const rawrbox::Vector2i& /*_mousePos*/) {}

	void UIContainer::key(uint32_t /*_key*/, uint32_t /*_scancode*/, uint32_t /*_action*/, uint32_t /*_mods*/) {}
	void UIContainer::keyChar(uint32_t /*_key*/) {}
	// ---

	void UIContainer::updateChildren() {
		this->internalUpdate(this);
	}

	void UIContainer::internalUpdate(rawrbox::UIContainer* elm) {
		if (elm == nullptr || !elm->visible()) return;
		elm->update();

		auto& elms = elm->getChildren();
		for (auto& celm : elms) {
			this->internalUpdate(celm.get());
		}
	}

	void UIContainer::update() {}
} // namespace rawrbox
