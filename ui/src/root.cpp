
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/ui/root.hpp>

namespace rawrbox {
	UIRoot::UIRoot(rawrbox::Window& window) : _window(&window) {
		this->_aabb = {0, 0, static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};

		// BINDS ---
		window.onChar += [this](Window& win, uint32_t character) mutable {
			if (this->focusedElement == nullptr) return;
			this->focusedElement->keyChar(character);
		};

		window.onKey += [this](Window& win, uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods) mutable {
			if (this->focusedElement == nullptr) return;
			this->focusedElement->key(key, scancode, action, mods);
		};

		window.onMouseKey += [this](Window& win, const rawrbox::Vector2i& location, uint32_t button, uint32_t action, uint32_t mods) mutable {
			this->onMousePress(location, button, action, mods);
		};

		window.onMouseMove += [this](Window& win, const rawrbox::Vector2i& location) mutable {
			this->onMouseMove(location);
		};

		window.onMouseScroll += [this](Window& win, const rawrbox::Vector2i& location, const rawrbox::Vector2i& offset) mutable {
			if (this->focusedElement == nullptr) return;
			this->focusedElement->mouseScroll(location, offset);
		};

		window.onResize += [this](Window& win, const rawrbox::Vector2i& size) mutable {
			this->_aabb = {0, 0, static_cast<float>(size.x), static_cast<float>(size.y)};
		};
		/// ----
	}

	// INTERNAL UTILS
	rawrbox::UIContainer* UIRoot::findElement(const rawrbox::Vector2i& mousePos, rawrbox::Vector2i& offsetOut) {
		auto& children = this->getChildren();
		for (size_t i = children.size(); i > 0; i--) {
			auto base = children[i - 1].get();
			if (base == nullptr) continue;

			auto elm = this->findElement(base, mousePos, {0, 0}, offsetOut);
			if (elm != nullptr) return elm;
		}

		return nullptr;
	}

	rawrbox::UIContainer* UIRoot::findElement(rawrbox::UIContainer* elmPtr, const rawrbox::Vector2i& mousePos, const rawrbox::Vector2i& offset, rawrbox::Vector2i& offsetOut) {
		if (elmPtr == nullptr || !elmPtr->visible() || !elmPtr->hitTest(mousePos.cast<float>())) return nullptr;

		auto pos = (elmPtr->getPos() + elmPtr->getDrawOffset()).cast<int>();
		auto& elms = elmPtr->getChildren();

		for (size_t i = elms.size(); i > 0; i--) {
			auto base = elms[i - 1].get();
			if (base == nullptr) continue;

			auto found = this->findElement(base, mousePos - pos, offset + pos, offsetOut);
			if (found != nullptr) return found;
		}

		offsetOut = offset + pos;
		return elmPtr;
	}
	// ---------------------

	// UTIL
	const rawrbox::AABBf& UIRoot::getAABB() const { return this->_aabb; }
	// ---

	// CHILDREN
	void UIRoot::removeChildren() {
		for (auto& ch : this->_children) {
			ch->remove();
		}

		this->_children.clear();
	}

	void UIRoot::removeChild(rawrbox::UIContainer* elm) {
		if (elm == nullptr) return;

		if (this->focusedElement == elm) this->focusedElement = nullptr;
		if (this->hoveredElement == elm) this->hoveredElement = nullptr;

		this->_children.erase(std::find_if(this->_children.begin(), this->_children.end(), [elm](auto& c) {
			return c.get() == elm;
		}));
	}

	std::vector<std::unique_ptr<rawrbox::UIContainer>>& UIRoot::getChildren() {
		return this->_children;
	}
	// -----

	// EVENTS ---
	void UIRoot::onMousePress(const rawrbox::Vector2i& location, uint32_t button, uint32_t action, uint32_t mods) {
		if (action == 0) {
			this->_pressingMouseButton--;

			if (this->focusedElement == nullptr) return;
			this->focusedElement->mouseUp(location - this->focusedElement->getPos().cast<int>(), button, mods);

			return;
		}

		this->_pressingMouseButton++;

		// Clicked outside, reset focus ---
		rawrbox::Vector2i offsetOut = {};
		auto target = this->findElement(location, offsetOut);
		if (target == nullptr) {
			this->focusedElement = nullptr;
			return;
		}
		//----------------

		this->setFocus(target);
		target->mouseDown(location - offsetOut, button, mods);
	}

	void UIRoot::onMouseMove(const rawrbox::Vector2i& location) {
		rawrbox::Vector2i offsetOut = {};
		auto target = findElement(location, offsetOut);

		// were holding our mouse on something, like dragging?
		rawrbox::UIContainer* focused = nullptr;
		if (this->_pressingMouseButton > 0 && this->focusedElement != nullptr) {
			focused = this->focusedElement;
		}

		// not hovering anything, so send it off to the scene event
		if (target == nullptr) {
			if (this->hoveredElement != nullptr) {
				this->hoveredElement->setHovering(false);
				this->hoveredElement = nullptr;
			}

			if (focused != nullptr) {
				auto absPos = focused->getPosAbsolute().cast<int>();
				focused->mouseMove(location - absPos);
			}

			return;
		}

		// see if we're having a different target, if so notify it
		if (this->hoveredElement != nullptr) {
			if (this->hoveredElement != target) {
				this->hoveredElement->setHovering(false);
				target->setHovering(true);
				this->hoveredElement = target;
			}
		} else {
			target->setHovering(true);
			this->hoveredElement = target;
		}

		target->mouseMove(location - offsetOut);

		// also send it to the dragged element if relevant
		if (focused != nullptr) {
			auto absPos = focused->getPosAbsolute().cast<int>();
			focused->mouseMove(location - absPos);
		}
	}
	// -----

	// FOCUS
	const rawrbox::UIContainer* UIRoot::getFocus() const {
		return this->focusedElement;
	}

	void UIRoot::setFocus(rawrbox::UIContainer* elm) {
		// Reset old focus & set new one
		if (this->focusedElement != nullptr) this->focusedElement->setFocused(false);

		elm->bringToFront();
		elm->setFocused(true);
		this->focusedElement = elm;
		// ---
	}
	// -----

	void UIRoot::render() {
		if (this->_window == nullptr) return;
		auto& sten = this->_window->getStencil();
		for (auto& ch : this->_children) {
			ch->drawChildren(sten);
		}
		sten.render();
	}

	void UIRoot::update() {
		if (this->_window == nullptr) return;
		for (auto& ch : this->_children) {
			ch->updateChildren();
		}
	}
} // namespace rawrbox
