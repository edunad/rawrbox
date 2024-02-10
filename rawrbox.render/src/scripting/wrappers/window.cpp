
#include <rawrbox/render/scripting/wrappers/window.hpp>

namespace rawrbox {
	WindowWrapper::WindowWrapper(rawrbox::Window* window) : _window(window) {}

	// Utils ----
	void WindowWrapper::setTitle(const std::string& title) {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		this->_window->setTitle(title);
	}

	void WindowWrapper::hideCursor(bool hidden) {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		this->_window->hideCursor(hidden);
	}

	void WindowWrapper::setCursor(uint32_t cursor) {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		this->_window->setCursor(cursor);
	}

	void WindowWrapper::setSize(const rawrbox::Vector2& /*size*/) const {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		// this->_window->setSize(size.cast<int>()); // TODO: THIS WILL AFFECT THE RENDERER, WE DON'T SUPPORT RESIZE FULLY YET
	}

	rawrbox::Vector2 WindowWrapper::getSize() const {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		return this->_window->getSize().cast<float>();
	}

	void WindowWrapper::setPos(const rawrbox::Vector2& pos) const {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		this->_window->setPos(pos.cast<int>());
	}

	rawrbox::Vector2 WindowWrapper::getPos() const {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		return this->_window->getPos().cast<float>();
	}

	float WindowWrapper::getAspectRatio() const {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		return this->_window->getAspectRatio();
	}

	rawrbox::Vector2 WindowWrapper::getMousePos() const {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		return this->_window->getMousePos().cast<float>();
	}

	bool WindowWrapper::isKeyDown(int key) const {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		return this->_window->isKeyDown(key);
	}

	bool WindowWrapper::isMouseDown(int key) const {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		return this->_window->isMouseDown(key);
	}

	void WindowWrapper::close() {
		if (this->_window == nullptr) throw std::runtime_error("Invalid window handle");
		this->_window->close();
	}
	// -------

	void WindowWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::WindowWrapper>("Window")
		    .addFunction("setTitle", &WindowWrapper::setTitle)

		    .addFunction("hideCursor", &WindowWrapper::hideCursor)
		    .addFunction("setCursor", &WindowWrapper::setCursor)

		    .addFunction("setSize", &WindowWrapper::setSize)
		    .addFunction("getSize", &WindowWrapper::getSize)

		    .addFunction("setPos", &WindowWrapper::setPos)
		    .addFunction("getPos", &WindowWrapper::getPos)

		    .addFunction("getAspectRatio", &WindowWrapper::getAspectRatio)
		    .addFunction("getMousePos", &WindowWrapper::getMousePos)

		    .addFunction("isKeyDown", &WindowWrapper::isKeyDown)
		    .addFunction("isMouseDown", &WindowWrapper::isMouseDown)

		    .addFunction("close", &WindowWrapper::close)
		    .endClass();
	}
} // namespace rawrbox
