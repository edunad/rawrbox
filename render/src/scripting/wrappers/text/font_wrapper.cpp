
#include <rawrbox/render/scripting/wrappers/text/font_wrapper.hpp>

namespace rawrbox {
	FontWrapper::FontWrapper(rawrbox::Font* instance) : _ref(instance) {}
	rawrbox::Font* FontWrapper::getRef() const { return this->_ref; }

	// UTILS -----
	bool FontWrapper::isValid() const { return this->_ref != nullptr; }

	float FontWrapper::getSize() const {
		if (!this->isValid()) return 0.F;
		return this->_ref->getSize();
	}

	float FontWrapper::getScale() const {
		if (!this->isValid()) return 0.F;
		return this->_ref->getScale();
	}

	float FontWrapper::getLineHeight() const {
		if (!this->isValid()) return 0.F;
		return this->_ref->getLineHeight();
	}

	rawrbox::Vector2f FontWrapper::getStringSize(const std::string& text) const {
		if (!this->isValid()) return {};
		return this->_ref->getStringSize(text);
	}

	bool FontWrapper::hasGlyph(uint32_t codepoint) const {
		if (!this->isValid()) return false;
		return this->_ref->hasGlyph(codepoint);
	}
	// ------

	void FontWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<FontWrapper>("Font",
		    sol::no_constructor,

		    // UTILS -----
		    "isValid", &FontWrapper::isValid,
		    "getSize", &FontWrapper::getSize,
		    "getScale", &FontWrapper::getScale,
		    "getLineHeight", &FontWrapper::getLineHeight,
		    "getStringSize", &FontWrapper::getStringSize,
		    "hasGlyph", &FontWrapper::hasGlyph
		    // ------
		);
	}
} // namespace rawrbox
