#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/input_wrapper.hpp>
#include <rawrbox/utils/memory.hpp>

namespace rawrbox {

	// UTILS -----
	void InputWrapper::setHints(const sol::table& hints) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setHints(rawrbox::LuaUtils::luaToVector<std::string>(hints));
	}

	void InputWrapper::setText(const std::string& text, sol::optional<bool> updateCharet, sol::optional<bool> preventEvent) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setText(text, updateCharet.value_or(false), preventEvent.value_or(false));
	}

	const std::string& InputWrapper::getText() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getText();
	}

	void InputWrapper::setPlaceholder(const std::string& text) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setPlaceholder(text);
	}

	const std::string& InputWrapper::getPlaceholder() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getPlaceholder();
	}

	void InputWrapper::setLimit(uint32_t limit) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setLimit(limit);
	}

	uint32_t InputWrapper::getLimit() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getLimit();
	}

	void InputWrapper::setFill(const std::string& fill) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setFill(fill);
	}

	void InputWrapper::setNumericOnly(bool numeric) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setNumericOnly(numeric);
	}

	bool InputWrapper::getNumericOnly() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getNumericOnly();
	}

	void InputWrapper::setReadOnly(bool read) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setReadOnly(read);
	}

	bool InputWrapper::getReadOnly() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getReadOnly();
	}

	void InputWrapper::setPadding(float padding) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setPadding(padding);
	}

	float InputWrapper::getPadding() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getPadding();
	}

	void InputWrapper::setColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setColor(col.cast<float>());
	}

	rawrbox::Colori InputWrapper::getColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getColor().cast<int>();
	}

	void InputWrapper::setBorderSize(float size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setBorderSize(size);
	}

	void InputWrapper::setBorderColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setBorderColor(col.cast<float>());
	}

	rawrbox::Colori InputWrapper::getBorderColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getBorderColor().cast<int>();
	}

	void InputWrapper::setBackgroundColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setBackgroundColor(col.cast<float>());
	}

	rawrbox::Colori InputWrapper::getBackgroundColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getBackgroundColor().cast<int>();
	}

	void InputWrapper::setFont(const rawrbox::FontWrapper& font) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setFont(font.getRef());
	}

	void InputWrapper::setFont(const std::string& font, sol::optional<uint16_t> size, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-InputWrapper] MOD not set!");
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");

		std::string modFolder = modEnv.env.value()["__mod_folder"];
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->setFont(rawrbox::LuaUtils::getContent(font, modFolder), size.value_or(11));
	}

	rawrbox::FontWrapper InputWrapper::getFont() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return {rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->getFont()};
	}

	bool InputWrapper::empty() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->empty();
	}

	size_t InputWrapper::size() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->size();
	}

	void InputWrapper::clear() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InputWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIInput>(this->_ref).lock()->clear();
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

		    "setFont", sol::overload(sol::resolve<void(const std::string&, sol::optional<uint16_t>, sol::this_environment)>(&InputWrapper::setFont), sol::resolve<void(const rawrbox::FontWrapper&)>(&InputWrapper::setFont)),
		    "getFont", &InputWrapper::getFont,

		    "empty", &InputWrapper::empty,
		    "size", &InputWrapper::size,
		    "clear", &InputWrapper::clear,

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
