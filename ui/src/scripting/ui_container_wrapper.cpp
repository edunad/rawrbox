#include <rawrbox/ui/container.hpp>
#include <rawrbox/ui/scripting/ui_container_wrapper.hpp>

namespace rawrbox {
	UIContainerWrapper::UIContainerWrapper(const std::shared_ptr<rawrbox::UIContainer>& ref) : _ref(ref) {}
	UIContainerWrapper::~UIContainerWrapper() { this->_ref.reset(); }

	// UTILS ---
	void UIContainerWrapper::setPos(const rawrbox::Vector2f& pos) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		this->_ref.lock()->setPos(pos);
	}

	const rawrbox::Vector2f UIContainerWrapper::getPos() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		return this->_ref.lock()->getPos();
	}

	const rawrbox::Vector2f UIContainerWrapper::getDrawOffset() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		return this->_ref.lock()->getDrawOffset();
	}

	void UIContainerWrapper::setSize(const rawrbox::Vector2f& size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		this->_ref.lock()->setSize(size);
	}

	const rawrbox::Vector2f UIContainerWrapper::getSize() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		return this->_ref.lock()->getSize();
	}

	void UIContainerWrapper::removeChildren() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		this->_ref.lock()->removeChildren();
	}

	void UIContainerWrapper::remove() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		this->_ref.lock()->remove();
	}

	void UIContainerWrapper::setVisible(bool visible) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		this->_ref.lock()->setVisible(visible);
	}

	bool UIContainerWrapper::visible() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		return this->_ref.lock()->visible();
	}
	// -----------

	// SORTING -----
	bool UIContainerWrapper::alwaysOnTop() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		return this->_ref.lock()->alwaysOnTop();
	}

	void UIContainerWrapper::setAlwaysTop(bool top) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		this->_ref.lock()->setAlwaysTop(top);
	}

	void UIContainerWrapper::bringToFront() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-UIContainer] Container reference not set!");
		this->_ref.lock()->bringToFront();
	}
	// -------

	bool UIContainerWrapper::isValid() const {
		return !this->_ref.expired();
	}

	void UIContainerWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<UIContainerWrapper>("UIContainer",
		    sol::no_constructor,

		    // UTILS ---
		    "setPos", &UIContainerWrapper::setPos,
		    "getPos", &UIContainerWrapper::getPos,
		    "getDrawOffset", &UIContainerWrapper::getDrawOffset,
		    "setSize", &UIContainerWrapper::setSize,
		    "getSize", &UIContainerWrapper::getSize,

		    "removeChildren", &UIContainerWrapper::removeChildren,
		    "remove", &UIContainerWrapper::remove,
		    "setVisible", &UIContainerWrapper::setVisible,
		    "visible", &UIContainerWrapper::visible,
		    // -----------

		    // SORTING -----
		    "alwaysOnTop", &UIContainerWrapper::alwaysOnTop,
		    "setAlwaysTop", &UIContainerWrapper::setAlwaysTop,
		    "bringToFront", &UIContainerWrapper::bringToFront,
		    // -----------

		    "isValid", &UIContainerWrapper::isValid);
	}
} // namespace rawrbox
