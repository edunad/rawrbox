#include <rawrbox/ui/elements/label.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/label_wrapper.hpp>

namespace rawrbox {
	LabelWrapper::LabelWrapper(const std::shared_ptr<rawrbox::UIContainer>& element, rawrbox::Mod* mod) : rawrbox::UIContainerWrapper(element), _mod(mod) {}

	// UTILS -----
	void LabelWrapper::setColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		ptr.lock()->setColor(col.cast<float>());
	}

	rawrbox::Colori LabelWrapper::getColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		return ptr.lock()->getColor().cast<int>();
	}

	void LabelWrapper::setShadowPos(const rawrbox::Vector2f& pos) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		ptr.lock()->setShadowPos(pos);
	}

	const rawrbox::Vector2f& LabelWrapper::getShadowPos() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		return ptr.lock()->getShadowPos();
	}

	void LabelWrapper::setShadowColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		ptr.lock()->setShadowColor(col.cast<float>());
	}

	rawrbox::Colori LabelWrapper::getShadowColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		return ptr.lock()->getShadowColor().cast<int>();
	}

	void LabelWrapper::setText(const std::string& text) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		ptr.lock()->setText(text);
	}

	const std::string& LabelWrapper::getText() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		return ptr.lock()->getText();
	}

	void LabelWrapper::setFont(const rawrbox::FontWrapper& font) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		ptr.lock()->setFont(font.getRef());
	}

	void LabelWrapper::setFont(const std::string& font, sol::optional<int> size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		ptr.lock()->setFont(rawrbox::LuaUtils::getContent(font, this->_mod->getFolder()), size.value_or(11));
	}

	rawrbox::FontWrapper LabelWrapper::getFont() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		return {ptr.lock()->getFont()};
	}

	void LabelWrapper::sizeToContents() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-LabelWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UILabel> ptr = std::dynamic_pointer_cast<rawrbox::UILabel>(this->_ref.lock());

		ptr.lock()->sizeToContents();
	}
	// ----

	void LabelWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<LabelWrapper>("UILabel",
		    sol::no_constructor,

		    "setColor", &LabelWrapper::setColor,
		    "getColor", &LabelWrapper::getColor,

		    "setShadowPos", &LabelWrapper::setShadowPos,
		    "getShadowPos", &LabelWrapper::getShadowPos,

		    "setShadowColor", &LabelWrapper::setShadowColor,
		    "getShadowColor", &LabelWrapper::getShadowColor,

		    "setText", &LabelWrapper::setText,
		    "getText", &LabelWrapper::getText,

		    "setFont", sol::overload(sol::resolve<void(const std::string&, sol::optional<int>)>(&LabelWrapper::setFont), sol::resolve<void(const rawrbox::FontWrapper&)>(&LabelWrapper::setFont)),
		    "getFont", &LabelWrapper::getFont,

		    "sizeToContents", &LabelWrapper::sizeToContents,

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
