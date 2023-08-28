#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/input_wrapper.hpp>

namespace rawrbox {
	InputWrapper::InputWrapper(const std::shared_ptr<rawrbox::UIContainer>& element, rawrbox::Mod* mod) : rawrbox::UIContainerWrapper(element), _mod(mod) {}

	// UTILS -----
	void InputWrapper::setHints(const sol::table& hints) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setHints(rawrbox::LuaUtils::luaToVector<std::string>(hints));
	}

	void InputWrapper::setText(const std::string& text, sol::optional<bool> updateCharet, sol::optional<bool> preventEvent) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setText(text, updateCharet.value_or(false), preventEvent.value_or(false));
	}

	const std::string& InputWrapper::getText() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getText();
	}

	void InputWrapper::setPlaceholder(const std::string& text) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setPlaceholder(text);
	}

	const std::string& InputWrapper::getPlaceholder() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getPlaceholder();
	}

	void InputWrapper::setLimit(uint32_t limit) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setLimit(limit);
	}

	uint32_t InputWrapper::getLimit() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getLimit();
	}

	void InputWrapper::setFill(const std::string& fill) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setFill(fill);
	}

	void InputWrapper::setNumericOnly(bool numeric) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setNumericOnly(numeric);
	}

	bool InputWrapper::getNumericOnly() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getNumericOnly();
	}

	void InputWrapper::setReadOnly(bool read) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setReadOnly(read);
	}

	bool InputWrapper::getReadOnly() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getReadOnly();
	}

	void InputWrapper::setPadding(float padding) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setPadding(padding);
	}

	float InputWrapper::getPadding() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getPadding();
	}

	void InputWrapper::setColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setColor(col.cast<float>());
	}

	rawrbox::Colori InputWrapper::getColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getColor().cast<int>();
	}

	void InputWrapper::setBorderSize(float size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setBorderSize(size);
	}

	void InputWrapper::setBorderColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setBorderColor(col.cast<float>());
	}

	rawrbox::Colori InputWrapper::getBorderColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getBorderColor().cast<int>();
	}

	void InputWrapper::setBackgroundColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setBackgroundColor(col.cast<float>());
	}

	rawrbox::Colori InputWrapper::getBackgroundColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->getBackgroundColor().cast<int>();
	}

	void InputWrapper::setFont(const rawrbox::FontWrapper& font) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setFont(font.getRef());
	}

	void InputWrapper::setFont(const std::string& font, sol::optional<int> size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->setFont(rawrbox::LuaUtils::getContent(font, this->_mod->getFolder()), size.value_or(11));
	}

	rawrbox::FontWrapper InputWrapper::getFont() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return {ptr.lock()->getFont()};
	}

	bool InputWrapper::empty() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->empty();
	}

	size_t InputWrapper::size() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		return ptr.lock()->size();
	}

	void InputWrapper::clear() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIInput> ptr = std::dynamic_pointer_cast<rawrbox::UIInput>(this->_ref.lock());

		ptr.lock()->clear();
	}

	// ----
	void InputWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<InputWrapper>("UIInput",
		    sol::no_constructor,

		    "setHints", &InputWrapper::setHints,

		    "setText", &InputWrapper::setText,
		    "getText", &InputWrapper::getText,

		    "setPlaceholder", &InputWrapper::setPlaceholder,
		    "getPlaceholder", &InputWrapper::getPlaceholder,

		    "setLimit", &InputWrapper::setLimit,
		    "getLimit", &InputWrapper::getLimit,

		    "setFill", &InputWrapper::setFill,

		    "setNumericOnly", &InputWrapper::setNumericOnly,
		    "getNumericOnly", &InputWrapper::getNumericOnly,

		    "setReadOnly", &InputWrapper::setReadOnly,
		    "getReadOnly", &InputWrapper::getReadOnly,

		    "setPadding", &InputWrapper::setPadding,
		    "getPadding", &InputWrapper::getPadding,

		    "setColor", &InputWrapper::setColor,
		    "getColor", &InputWrapper::getColor,

		    "setBorderSize", &InputWrapper::setBorderSize,
		    "setBorderColor", &InputWrapper::setBorderColor,
		    "getBorderColor", &InputWrapper::getBorderColor,

		    "setBackgroundColor", &InputWrapper::setBackgroundColor,
		    "getBackgroundColor", &InputWrapper::getBackgroundColor,

		    "setFont", sol::overload(sol::resolve<void(const std::string&, sol::optional<int>)>(&InputWrapper::setFont), sol::resolve<void(const rawrbox::FontWrapper&)>(&InputWrapper::setFont)),
		    "getFont", &InputWrapper::getFont,

		    "empty", &InputWrapper::empty,
		    "size", &InputWrapper::size,
		    "clear", &InputWrapper::clear,

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
