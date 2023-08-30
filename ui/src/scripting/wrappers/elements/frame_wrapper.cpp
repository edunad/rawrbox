#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/frame_wrapper.hpp>
#include <rawrbox/utils/memory.hpp>

namespace rawrbox {
	void FrameWrapper::setTitle(const std::string& title) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->setTitle(title);
	}

	const std::string& FrameWrapper::getTitle() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->getTitle();
	}

	rawrbox::Colori FrameWrapper::getTitleColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->getTitleColor().cast<int>();
	}

	void FrameWrapper::setTitleColor(const rawrbox::Colori& cl) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->setTitleColor(cl.cast<float>());
	}

	void FrameWrapper::setDraggable(bool enabled) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->setDraggable(enabled);
	}

	bool FrameWrapper::isDraggable() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->isDraggable();
	}

	void FrameWrapper::setClosable(bool enabled) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->setClosable(enabled);
	}

	bool FrameWrapper::isClosable() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->isClosable();
	}

	void FrameWrapper::onClose(sol::function onCloseCallback) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIFrame>(this->_ref).lock()->onClose += [onCloseCallback] {
			return rawrbox::LuaUtils::runCallback(onCloseCallback);
		};
	}

	void FrameWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<FrameWrapper>("UIFrame",
		    sol::no_constructor,

		    "setTitle", &FrameWrapper::setTitle,
		    "getTitle", &FrameWrapper::getTitle,

		    "setTitleColor", &FrameWrapper::setTitleColor,
		    "getTitleColor", &FrameWrapper::getTitleColor,

		    "setDraggable", &FrameWrapper::setDraggable,
		    "isDraggable", &FrameWrapper::isDraggable,

		    "setClosable", &FrameWrapper::setClosable,
		    "isClosable", &FrameWrapper::isClosable,

		    "onClose", &FrameWrapper::onClose,
		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
