#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/button_wrapper.hpp>

#include <fmt/format.h>

namespace rawrbox {
	ButtonWrapper::ButtonWrapper(const std::shared_ptr<rawrbox::UIContainer>& element, rawrbox::Mod* mod) : rawrbox::UIContainerWrapper(element), _mod(mod) {}

	void ButtonWrapper::setTextureSize(const rawrbox::Vector2i& size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setTextureSize(size.cast<float>());
	}

	void ButtonWrapper::setTextureColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setTextureColor(col.cast<float>());
	}

	rawrbox::Colori ButtonWrapper::getTextureColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		return ptr.lock()->getTextureColor().cast<int>();
	}

	void ButtonWrapper::setTextColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setTextColor(col.cast<float>());
	}

	rawrbox::Colori ButtonWrapper::getTextColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		return ptr.lock()->getTextColor().cast<int>();
	}

	void ButtonWrapper::setBackgroundColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setBackgroundColor(col.cast<float>());
	}

	rawrbox::Colori ButtonWrapper::getBackgroundColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		return ptr.lock()->getBackgroundColor().cast<int>();
	}

	void ButtonWrapper::setText(const std::string& text, sol::optional<int> size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setText(text, size.value_or(16));
	}

	const std::string& ButtonWrapper::getText() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		return ptr.lock()->getText();
	}

	void ButtonWrapper::setTooltip(const std::string& text) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setTooltip(text);
	}

	const std::string& ButtonWrapper::getTooltip() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		return ptr.lock()->getTooltip();
	}

	void ButtonWrapper::setTexture(const rawrbox::TextureWrapper& texture) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setTexture(texture.getRef());
	}

	void ButtonWrapper::setTexture(const std::string& path) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setTexture(rawrbox::LuaUtils::getContent(path, this->_mod->getFolder()));
	}

	void ButtonWrapper::setEnabled(bool enabled) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setEnabled(enabled);
	}

	bool ButtonWrapper::isEnabled() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		return ptr.lock()->isEnabled();
	}

	void ButtonWrapper::setBorder(bool enabled) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->setBorder(enabled);
	}

	bool ButtonWrapper::borderEnabled() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		return ptr.lock()->borderEnabled();
	}

	void ButtonWrapper::onClick(sol::function callback) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIButton> ptr = std::dynamic_pointer_cast<rawrbox::UIButton>(this->_ref.lock());

		ptr.lock()->onClick += [callback] {
			return rawrbox::LuaUtils::runCallback(callback);
		};
	}

	void ButtonWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ButtonWrapper>("UIButton",
		    sol::no_constructor,

		    "setTextureSize", &ButtonWrapper::setTextureSize,

		    "setTextureColor", &ButtonWrapper::setTextureColor,
		    "getTextureColor", &ButtonWrapper::getTextureColor,

		    "setTextColor", &ButtonWrapper::setTextColor,
		    "getTextColor", &ButtonWrapper::getTextColor,

		    "setBackgroundColor", &ButtonWrapper::setBackgroundColor,
		    "getBackgroundColor", &ButtonWrapper::getBackgroundColor,

		    "setText", &ButtonWrapper::setText,
		    "getText", &ButtonWrapper::getText,

		    "setTooltip", &ButtonWrapper::setTooltip,
		    "getTooltip", &ButtonWrapper::getTooltip,

		    "setTexture", sol::overload(sol::resolve<void(const std::string&)>(&ButtonWrapper::setTexture), sol::resolve<void(const rawrbox::TextureWrapper&)>(&ButtonWrapper::setTexture)),

		    "setEnabled", &ButtonWrapper::setEnabled,
		    "isEnabled", &ButtonWrapper::isEnabled,

		    "setBorder", &ButtonWrapper::setBorder,
		    "borderEnabled", &ButtonWrapper::borderEnabled,

		    "onClick", &ButtonWrapper::onClick,

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
