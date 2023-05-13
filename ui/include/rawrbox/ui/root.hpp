#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/aabb.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/ui/base.hpp>
#include <rawrbox/ui/container.hpp>

#include <array>
#include <memory>

namespace rawrbox {
	class UIRoot {
	protected:
		rawrbox::Window* _window = nullptr;
		std::vector<std::shared_ptr<rawrbox::UIContainer>> _uiElements = {};

		rawrbox::AABBi _size = {};

		std::weak_ptr<rawrbox::UIBase> _focusedElement;
		std::weak_ptr<rawrbox::UIBase> _hoveredElement;

		// INTERNAL UTILS
		int _pressingMouseButton = 0;

		std::shared_ptr<rawrbox::UIBase> findElement(const rawrbox::Vector2f& mousePos, rawrbox::Vector2f& offsetOut) {
			for (size_t i = this->_uiElements.size(); i > 0; i--) {
				auto base = std::dynamic_pointer_cast<rawrbox::UIBase>(this->_uiElements[i - 1]);
				if (base == nullptr) continue;

				auto elm = this->findElement(base, mousePos, {0, 0}, offsetOut);
				if (elm != nullptr) return elm;
			}

			return nullptr;
		}

		std::shared_ptr<rawrbox::UIBase> findElement(std::shared_ptr<rawrbox::UIBase> elmPtr, const rawrbox::Vector2f& mousePos, const rawrbox::Vector2f& offset, rawrbox::Vector2f& offsetOut) {
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
		// ----

		// EVENTS ---
		void onMousePress(const rawrbox::Vector2i& location, uint32_t button, uint32_t action, uint32_t mods) {
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

		void onMouseMove(const rawrbox::Vector2i& location) {
			rawrbox::Vector2f offsetOut = {};
			auto target = findElement(location.cast<float>(), offsetOut);

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

	public:
		virtual ~UIRoot() {
			this->_window = nullptr;
			this->_uiElements.clear();

			this->_focusedElement.reset();
			this->_hoveredElement.reset();
		}

		explicit UIRoot(rawrbox::Window* window) : _window(window), _size(0, 0, window->getSize().x, window->getSize().y) {
			// BINDS ---
			window->onChar += [this](Window& win, uint32_t character) mutable {
				rawrbox::runOnMainThread([this, character]() {
					if (this->_focusedElement.expired()) return;
					this->_focusedElement.lock()->keyChar(character);
				});
			};

			window->onKey += [this](Window& win, uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods) mutable {
				rawrbox::runOnMainThread([this, key, scancode, action, mods]() {
					if (this->_focusedElement.expired()) return;
					this->_focusedElement.lock()->key(key, scancode, action, mods);
				});
			};

			window->onMouseKey += [this](Window& win, const rawrbox::Vector2i& location, uint32_t button, uint32_t action, uint32_t mods) mutable {
				rawrbox::runOnMainThread([this, location, button, action, mods]() {
					this->onMousePress(location, button, action, mods);
				});
			};

			window->onMouseMove += [this](Window& win, const rawrbox::Vector2i& location) mutable {
				rawrbox::runOnMainThread([this, location]() {
					this->onMouseMove(location);
				});
			};

			window->onMouseScroll += [this](Window& win, const rawrbox::Vector2i& location, const rawrbox::Vector2i& offset) mutable {
				rawrbox::runOnMainThread([this, location, offset]() {
					if (this->_focusedElement.expired()) return;
					this->_focusedElement.lock()->mouseScroll(location, offset);
				});
			};
			/// ----
		}

		std::shared_ptr<rawrbox::UIBase> getFocus() {
			if (this->_focusedElement.expired()) return nullptr;
			return this->_focusedElement.lock();
		}

		void setFocus(std::shared_ptr<rawrbox::UIBase> elm) {
			// Reset old focus & set new one
			if (!this->_focusedElement.expired()) this->_focusedElement.lock()->setFocused(false);

			// target->bringToFront();
			elm->setFocused(true);
			this->_focusedElement = elm;
			// ---
		}

		void update() {
			for (auto elm : this->_uiElements) {
				elm->update();
			}
		}

		void draw(rawrbox::Stencil& stencil) {
			for (auto elm : this->_uiElements) {
				elm->draw(stencil);
			}
		}
	};
} // namespace rawrbox
