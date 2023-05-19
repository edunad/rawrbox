
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/ui/root.hpp>

namespace rawrbox {

	// INTERNAL UTILS
	std::shared_ptr<rawrbox::UIBase> UIRoot::findElement(const rawrbox::Vector2i& mousePos, rawrbox::Vector2i& offsetOut) {
		for (size_t i = this->getChildren().size(); i > 0; i--) {
			auto base = std::dynamic_pointer_cast<rawrbox::UIBase>(this->getChildren()[i - 1]);
			if (base == nullptr) continue;

			auto elm = this->findElement(base, mousePos, {0, 0}, offsetOut);
			if (elm != nullptr) return elm;
		}

		return nullptr;
	}

	std::shared_ptr<rawrbox::UIBase> UIRoot::findElement(std::shared_ptr<rawrbox::UIBase> elmPtr, const rawrbox::Vector2i& mousePos, const rawrbox::Vector2i& offset, rawrbox::Vector2i& offsetOut) {
		if (elmPtr == nullptr || !elmPtr->visible() || !elmPtr->hitTest(mousePos.cast<float>())) return nullptr;

		auto pos = elmPtr->getPos().cast<int>();
		auto elms = elmPtr->getChildren();

		for (size_t i = elms.size(); i > 0; i--) {
			auto base = elms[i - 1];
			if (base == nullptr) continue;

			auto found = this->findElement(base, mousePos - pos, offset + pos, offsetOut);
			if (found != nullptr) return found;
		}

		offsetOut = offset + pos;
		return elmPtr;
	}
	// ---------------------

	// EVENTS ---
	void UIRoot::onMousePress(const rawrbox::Vector2i& location, uint32_t button, uint32_t action, uint32_t mods) {
		if (action == 0) {
			this->_pressingMouseButton--;

			if (this->_focusedElement.expired()) return;

			auto focused = this->_focusedElement.lock();
			focused->mouseUp(location - focused->getPos().cast<int>(), button, mods);
			return;
		}

		this->_pressingMouseButton++;

		// Clicked outside, reset focus ---
		rawrbox::Vector2i offsetOut = {};
		auto target = this->findElement(location, offsetOut);
		if (target == nullptr) {
			this->_focusedElement.reset();
			return;
		}
		//----------------

		this->setFocus(target);
		target->mouseDown(location - offsetOut, button, mods);
	}

	void UIRoot::onMouseMove(const rawrbox::Vector2i& location) {
		rawrbox::Vector2i offsetOut = {};
		auto target = this->findElement(location, offsetOut);

		// were holding our mouse on something, like dragging?
		std::shared_ptr<rawrbox::UIBase> focused = nullptr;
		if (this->_pressingMouseButton > 0 && !this->_focusedElement.expired()) {
			focused = this->_focusedElement.lock();
		}

		// not hovering anything, so send it off to the scene event
		if (target == nullptr) {
			if (!this->_hoveredElement.expired()) {
				auto elm = this->_hoveredElement.lock();
				elm->setHovering(false);

				this->_hoveredElement.reset();
			}

			if (focused != nullptr) {
				auto absPos = focused->getPosAbsolute().cast<int>();
				focused->mouseMove(location - absPos);
			}

			return;
		}

		// see if we're having a different target, if so notify it
		if (!this->_hoveredElement.expired()) {
			auto elm = this->_hoveredElement.lock();
			if (elm != target) {
				elm->setHovering(false);
				target->setHovering(true);
				this->_hoveredElement = target;
			}
		} else {
			target->setHovering(true);
			this->_hoveredElement = target;
		}

		target->mouseMove(location - offsetOut);

		// also send it to the dragged element if relevant
		if (focused != nullptr) {
			auto absPos = focused->getPosAbsolute().cast<int>();
			focused->mouseMove(location - absPos);
		}
	}
	// -----

	UIRoot::~UIRoot() {
		this->removeChildren();

		this->_focusedElement.reset();
		this->_hoveredElement.reset();
	}

	std::shared_ptr<UIRoot> UIRoot::create(rawrbox::Window& window) {
		auto ret = std::make_shared<rawrbox::UIRoot>();
		ret->setRef(ret);

		ret->_aabb = {0, 0, static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};

		// BINDS ---
		window.onChar += [ret](Window& win, uint32_t character) mutable {
			rawrbox::runOnMainThread([ret, character]() {
				if (ret->_focusedElement.expired()) return;
				ret->_focusedElement.lock()->keyChar(character);
			});
		};

		window.onKey += [ret](Window& win, uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods) mutable {
			rawrbox::runOnMainThread([ret, key, scancode, action, mods]() {
				if (ret->_focusedElement.expired()) return;
				ret->_focusedElement.lock()->key(key, scancode, action, mods);
			});
		};

		window.onMouseKey += [ret](Window& win, const rawrbox::Vector2i& location, uint32_t button, uint32_t action, uint32_t mods) mutable {
			rawrbox::runOnMainThread([ret, location, button, action, mods]() {
				ret->onMousePress(location, button, action, mods);
			});
		};

		window.onMouseMove += [ret](Window& win, const rawrbox::Vector2i& location) mutable {
			rawrbox::runOnMainThread([ret, location]() {
				ret->onMouseMove(location);
			});
		};

		window.onMouseScroll += [ret](Window& win, const rawrbox::Vector2i& location, const rawrbox::Vector2i& offset) mutable {
			rawrbox::runOnMainThread([ret, location, offset]() {
				if (ret->_focusedElement.expired()) return;
				ret->_focusedElement.lock()->mouseScroll(location, offset);
			});
		};
		/// ----

		return ret;
	}

	const std::shared_ptr<rawrbox::UIBase> UIRoot::getFocus() const {
		if (this->_focusedElement.expired()) return nullptr;
		return this->_focusedElement.lock();
	}

	void UIRoot::setFocus(std::shared_ptr<rawrbox::UIBase> elm) {
		// Reset old focus & set new one
		if (!this->_focusedElement.expired()) this->_focusedElement.lock()->setFocused(false);

		// target->bringToFront();
		elm->setFocused(true);
		this->_focusedElement = elm;
		// ---
	}
} // namespace rawrbox
