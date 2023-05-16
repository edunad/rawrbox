
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/ui/root.hpp>

namespace rawrbox {

	// INTERNAL UTILS
	std::shared_ptr<rawrbox::UIBase> UIRoot::findElement(const rawrbox::Vector2f& mousePos, rawrbox::Vector2f& offsetOut) {
		for (size_t i = this->getChildren().size(); i > 0; i--) {
			auto base = std::dynamic_pointer_cast<rawrbox::UIBase>(this->getChildren()[i - 1]);
			if (base == nullptr) continue;

			auto elm = this->findElement(base, mousePos, {0, 0}, offsetOut);
			if (elm != nullptr) return elm;
		}

		return nullptr;
	}

	std::shared_ptr<rawrbox::UIBase> UIRoot::findElement(std::shared_ptr<rawrbox::UIBase> elmPtr, const rawrbox::Vector2f& mousePos, const rawrbox::Vector2f& offset, rawrbox::Vector2f& offsetOut) {
		if (elmPtr == nullptr || !elmPtr->visible() || !elmPtr->hitTest(mousePos)) return nullptr;

		auto& pos = elmPtr->getPos();
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
			this->_focusedElement.lock()->mouseUp(location, button, mods);
			return;
		}

		this->_pressingMouseButton++;

		// Clicked outside, reset focus ---
		rawrbox::Vector2f offsetOut = {};
		auto target = this->findElement(location.cast<float>(), offsetOut);
		if (target == nullptr) {
			this->_focusedElement.reset();
			return;
		}
		//----------------

		this->setFocus(target);
		target->mouseDown(location, button, mods);
	}

	void UIRoot::onMouseMove(const rawrbox::Vector2i& location) {
		rawrbox::Vector2f offsetOut = {};
		auto target = this->findElement(location.cast<float>(), offsetOut);

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

			if (focused != nullptr) focused->mouseMove(location);
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
