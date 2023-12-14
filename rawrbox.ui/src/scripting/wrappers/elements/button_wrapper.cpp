#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/button_wrapper.hpp>
#include <rawrbox/utils/memory.hpp>

namespace rawrbox {

	void ButtonWrapper::setTextureSize(const rawrbox::Vector2i& size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setTextureSize(size.cast<float>());
	}

	void ButtonWrapper::setTextureColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setTextureColor(col.cast<float>());
	}

	rawrbox::Colori ButtonWrapper::getTextureColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->getTextureColor().cast<int>();
	}

	void ButtonWrapper::setTextColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setTextColor(col.cast<float>());
	}

	rawrbox::Colori ButtonWrapper::getTextColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->getTextColor().cast<int>();
	}

	void ButtonWrapper::setBackgroundColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setBackgroundColor(col.cast<float>());
	}

	rawrbox::Colori ButtonWrapper::getBackgroundColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->getBackgroundColor().cast<int>();
	}

	void ButtonWrapper::setText(const std::string& text, sol::optional<uint16_t> size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setText(text, size.value_or(16));
	}

	const std::string& ButtonWrapper::getText() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->getText();
	}

	void ButtonWrapper::setTooltip(const std::string& text) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setTooltip(text);
	}

	const std::string& ButtonWrapper::getTooltip() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->getTooltip();
	}

	void ButtonWrapper::setTexture(const rawrbox::TextureWrapper& texture) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setTexture(texture.getRef());
	}

	void ButtonWrapper::setTexture(const std::string& path, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-ButtonWrapper] MOD not set!");
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");

		std::string modFolder = modEnv.env.value()["__mod_folder"];
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setTexture(rawrbox::LuaUtils::getContent(path, modFolder));
	}

	void ButtonWrapper::setEnabled(bool enabled) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setEnabled(enabled);
	}

	bool ButtonWrapper::isEnabled() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->isEnabled();
	}

	void ButtonWrapper::setBorder(bool enabled) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->setBorder(enabled);
	}

	bool ButtonWrapper::borderEnabled() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ButtonWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->borderEnabled();
	}

	void ButtonWrapper::onClick(sol::function callback) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-FrameWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIButton>(this->_ref).lock()->onClick += [callback] {
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

		    "setTexture", sol::overload(sol::resolve<void(const std::string&, sol::this_environment)>(&ButtonWrapper::setTexture), sol::resolve<void(const rawrbox::TextureWrapper&)>(&ButtonWrapper::setTexture)),

		    "setEnabled", &ButtonWrapper::setEnabled,
		    "isEnabled", &ButtonWrapper::isEnabled,

		    "setBorder", &ButtonWrapper::setBorder,
		    "borderEnabled", &ButtonWrapper::borderEnabled,

		    "onClick", &ButtonWrapper::onClick,

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
