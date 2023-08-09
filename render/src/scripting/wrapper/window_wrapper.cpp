#include <rawrbox/render/scripting/wrapper/window_wrapper.hpp>

namespace rawrbox {
	WindowWrapper::WindowWrapper(rawrbox::Window* window) : _window(window) {}

	// Utils ----
	void WindowWrapper::setTitle(const std::string& title) {
		if (this->_window != nullptr) this->_window->setTitle(title);
	}

	void WindowWrapper::hideCursor(bool hidden) {
		if (this->_window != nullptr) this->_window->hideCursor(hidden);
	}

	void WindowWrapper::setCursor(uint32_t cursor) {
		if (this->_window != nullptr) this->_window->setCursor(cursor);
	}

	void WindowWrapper::close() {
		if (this->_window != nullptr) this->_window->close();
	}

	rawrbox::Vector2i WindowWrapper::getSize() const {
		if (this->_window != nullptr) return this->_window->getSize();
		return {};
	}

	float WindowWrapper::getAspectRatio() const {
		if (this->_window != nullptr) return this->_window->getAspectRatio();
		return 0.F;
	}

	rawrbox::Vector2i WindowWrapper::getMousePos() const {
		if (this->_window != nullptr) return this->_window->getMousePos();
		return {};
	}

	bool WindowWrapper::isKeyDown(int key) const {
		if (this->_window != nullptr) return this->_window->isKeyDown(key);
		return false;
	}

	bool WindowWrapper::isMouseDown(int key) const {
		if (this->_window != nullptr) return this->_window->isMouseDown(key);
		return false;
	}
	// -------

	void WindowWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<WindowWrapper>("Window",
		    sol::no_constructor,

		    // UTILS ---
		    "setTitle", &WindowWrapper::setTitle,
		    "hideCursor", &WindowWrapper::hideCursor,
		    "setCursor", &WindowWrapper::setCursor,
		    "close", &WindowWrapper::close,
		    "getSize", &WindowWrapper::getSize,
		    "getAspectRatio", &WindowWrapper::getAspectRatio,
		    "getMousePos", &WindowWrapper::getMousePos,
		    "isKeyDown", &WindowWrapper::isKeyDown,
		    "isMouseDown", &WindowWrapper::isMouseDown
		    // ---
		);
	}
} // namespace rawrbox
