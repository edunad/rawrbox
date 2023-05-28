
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/string.hpp>

#include <clip.h>

namespace rawrbox {
	UIInput::~UIInput() {
		this->_font.reset();
	}

	// INTERNAL ---
	void UIInput::genFill() {
		this->_fillText = "";
		if (this->_fillTextPattern.empty()) return;

		auto& size = this->getSize();
		float x = this->_textSize.x;

		while (true) {
			x += this->_fillSizeChar.x;
			if (x >= size.x) break;
			this->_fillText += this->_fillTextPattern;
		}
	}

	void UIInput::combo_paste() {
		if (this->_readOnly || this->_font.expired()) return;

		std::string paste_text = "";
		clip::get_text(paste_text);

		if (this->_numeric) paste_text = rawrbox::StrUtils::extractNumbers(paste_text);
		if (paste_text.empty()) return;

		auto bytePos = Font::getByteCount(getText(), this->_charXPos);

		// add the result UTF8 to our buffer
		auto curStr = this->getText();

		this->setText(curStr.insert(bytePos, paste_text));
		this->_charXPos += Font::getCharacterCount(paste_text);

		// be sure the font contains the letters typed
		this->_font.lock()->addChars(paste_text);
	}

	void UIInput::combo_backspace(uint32_t mods) {
		if (this->_readOnly) return;

		auto text = getText();
		if (text.empty() || this->_charXPos == 0) return;

		// If pressing ctrl we're removing text in chunks rather than per letter
		if ((mods & KEY_MODIFIER_CTRL) != 0) {
			auto bytePos = Font::getByteCount(text, this->_charXPos);
			auto tmp = text.substr(0, bytePos);

			size_t found = tmp.find_last_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"); // TODO: SUPPORT UTF8
			if (found == std::string::npos) {
				auto erasePos = Font::getByteCount(text, this->_charXPos);
				text.erase(text.begin(), text.begin() + erasePos);
				this->_charXPos = 0;
			} else {
				auto foundPos = Font::getByteCount(text, this->_charXPos - 1);
				if (found == foundPos) found--;

				auto oldChars = Font::getCharacterCount(text);
				text.erase(text.begin() + found + 1, text.begin() + Font::getByteCount(text, this->_charXPos));
				this->_charXPos -= oldChars - Font::getCharacterCount(text);
			}
		} else {
			auto fromPos = Font::getByteCount(text, this->_charXPos - 1);
			auto toPos = Font::getByteCount(text, this->_charXPos);

			text.erase(text.begin() + fromPos, text.begin() + toPos);
			this->moveCharet(false);
		}

		this->setText(text);
	}

	void UIInput::combo_delete(uint32_t mods) {
		if (this->_readOnly) return;

		auto text = getText();
		if (text.empty() || this->_charXPos == Font::getCharacterCount(text)) return;

		// If pressing ctrl we're removing text in chunks rather than per letter
		if ((mods & KEY_MODIFIER_CTRL) != 0) {
			auto offset = Font::getByteCount(text, this->_charXPos);
			auto start = text.begin() + offset;

			auto found = text.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", offset); // TODO: SUPPORT UTF8
			if (found == std::string::npos) {
				text.erase(start, text.end());
			} else {
				if (found == this->_charXPos) found++;
				text.erase(start, text.begin() + found);
			}
		} else {
			auto fromPos = Font::getByteCount(text, this->_charXPos);
			auto toPos = Font::getByteCount(text, this->_charXPos + 1);

			text.erase(text.begin() + fromPos, text.begin() + toPos);
		}

		setText(text);
	}

	void UIInput::moveCharet(bool forward) {
		if (this->_readOnly) return;
		int xPos = static_cast<int>(this->_charXPos);
		int charCount = static_cast<int>(Font::getCharacterCount(this->_text));
		int p = std::clamp<int>(forward ? xPos + 1 : xPos - 1, 0, charCount);

		this->_charXPos = static_cast<size_t>(p);
	}
	// -----

	// UTILS ----
	void UIInput::setHints(const std::vector<std::string>& hints) {
		this->_hint = fmt::format("{}", fmt::join(hints, ", "));
	}

	void UIInput::setText(const std::string& text, bool updateCharet, bool preventEvent) {
		if (this->_font.expired()) return;

		if (this->_limit > 0 && rawrbox::Font::getCharacterCount(text) > this->_limit) {
			this->_text = text.substr(0, Font::getByteCount(text, this->_limit));
		} else {
			this->_text = text;
		}

		this->_textSize = this->_font.lock()->getStringSize(this->_text);
		if (this->_text.empty()) {
			this->_textSize.y = this->_charSize.y; // Prevent setting y to 0
		} else if (updateCharet) {
			this->_charXPos = static_cast<int>(rawrbox::Font::getCharacterCount(this->_text)); // Set at the end
		}

		if (!preventEvent) onTextUpdate();
		this->genFill();
	}

	const std::string& UIInput::getText() const { return this->_text; }

	void UIInput::setPlaceholder(const std::string& text) { this->_placeholder = text; }
	const std::string& UIInput::getPlaceholder() const { return this->_placeholder; }

	void UIInput::setLimit(uint32_t limit) { this->_limit = limit; }
	uint32_t UIInput::getLimit() const { return this->_limit; }

	void UIInput::setFill(const std::string& fill) {
		if (this->_font.expired()) return;

		this->_fillTextPattern = fill;
		this->_fillSizeChar = this->_font.lock()->getStringSize(fill);

		this->genFill();
	}

	void UIInput::setNumericOnly(bool numeric) { this->_numeric = numeric; }
	bool UIInput::getNumericOnly() const { return this->_numeric; }

	void UIInput::setReadOnly(bool read) { this->_readOnly = read; }
	bool UIInput::getReadOnly() const { return this->_readOnly; }

	void UIInput::setPadding(float padding) { this->_padding = padding; }
	float UIInput::getPadding() const { return this->_padding; }

	void UIInput::setColor(const Color& col) { this->_textColor = col; }
	const Color& UIInput::getColor() const { return this->_textColor; }

	void UIInput::setBorderSize(float size) { this->_borderSize = size; }
	void UIInput::setBorderColor(const Color& col) { this->_border = col; }
	const rawrbox::Color& UIInput::getBorderColor() const { return this->_border; }

	void UIInput::setBackgroundColor(const rawrbox::Color& col) { this->_backgroundColor = col; }
	const Color& UIInput::getBackgroundColor() const { return this->_backgroundColor; }

	void UIInput::setFont(const std::string& font, int size) {
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>(font)->getSize(size);

		this->_charSize = this->_font.lock()->getStringSize("W");       // Tallest character possible, usually it's W
		if (this->_text.empty()) this->_textSize.y = this->_charSize.y; // Prevent setting y to 0
	}

	void UIInput::setFont(std::shared_ptr<rawrbox::Font> font) {
		if (font == nullptr) throw std::runtime_error("[RawrBox-UI] Invalid font");

		this->_font = font;
		this->_charSize = font->getStringSize("W");                     // Tallest character possible, usually it's W
		if (this->_text.empty()) this->_textSize.y = this->_charSize.y; // Prevent setting y to 0
	}

	std::weak_ptr<rawrbox::Font> UIInput::getFont() const { return this->_font; }

	size_t UIInput::size() const { return this->_text.size(); }
	bool UIInput::empty() const { return this->_text.empty(); }
	void UIInput::clear() {
		this->_charXPos = 0;
		this->setText("");
	}
	// ----------

	// INPUT ----
	void UIInput::mouseDown(const rawrbox::Vector2i& mousePos, uint32_t /*button*/, uint32_t /*mods*/) {
		if (this->_readOnly || this->_font.expired()) return;

		auto& text = this->getText();
		auto characterCount = Font::getCharacterCount(text);

		auto fnt = this->_font.lock();
		for (size_t i = 1; i < characterCount; i++) {
			auto size = fnt->getStringSize(text.substr(0, Font::getByteCount(text, i)));
			if (mousePos.x < size.cast<int>().x) {
				this->_charXPos = i - 1;
				return;
			}
		}

		this->_charXPos = characterCount;
	}

	void UIInput::keyChar(uint32_t key) {
		if (this->_readOnly || this->_font.expired() || key == KEY_GRAVE_ACCENT) return; // Ignore `, it's used to open stuff, why would you use this
		if (this->_numeric && ((key < KEY_0 || key > KEY_9) && (key < KEY_KP_0 || key > KEY_KP_9))) return;

		// check if we're going past our limit
		if (this->_limit > 0 && Font::getCharacterCount(this->_text) + 1 > this->_limit) return;

		// convert input key to UTF8
		auto letter = static_cast<wchar_t>(key);
		std::wstring wstr;
		wstr += letter;

		auto utf8 = Font::toUTF8(wstr);

		// insert our new uf8 letter into our existing utf8 string
		auto oldtext = this->_text;
		auto bytePos = Font::getByteCount(oldtext, this->_charXPos);

		this->setText(oldtext.insert(bytePos, utf8));
		this->moveCharet(true);

		// be sure the font contains the letters typed
		this->_font.lock()->addChars(utf8);
	}

	void UIInput::key(uint32_t key, uint32_t /*scancode*/, uint32_t action, uint32_t mods) {
		if (this->_readOnly || action == KEY_ACTION_UP) return;

		if (key == KEY_V && (mods & KEY_MODIFIER_CTRL) != 0) {
			this->combo_paste();
			return;
		} // ctrl V

		if (key == KEY_ENTER || key == KEY_KP_ENTER) {
			this->onEnter();
			return;
		}

		if (key == KEY_ESCAPE) {
			this->clear();
			return;
		}

		if (key == KEY_BACKSPACE) {
			this->combo_backspace(mods);
			return;
		}

		if (key == KEY_DELETE) {
			this->combo_delete(mods);
			return;
		}

		if (key == KEY_HOME) {
			this->_charXPos = 0;
		}

		if (key == KEY_END) {
			this->_charXPos = static_cast<int>(Font::getCharacterCount(getText()));
		}

		if (key == KEY_RIGHT) {
			this->moveCharet(true);
			return;
		}
		if (key == KEY_LEFT) {
			this->moveCharet(false);
			return;
		}

		this->onKey(key);
	}
	// ----------

	// FOCUS HANDLE --
	bool UIInput::lockKeyboard() const {
		return !this->_readOnly; // Prevent player movement while focused
	}
	// ---

	// DRAW ----
	void UIInput::draw(Stencil& stencil) {
		auto font = this->getFont();
		if (font.expired()) return;

		auto& size = this->getSize();
		auto& text = this->getText();
		auto fnt = font.lock();

		stencil.drawBox({}, size, focused() || this->_readOnly ? getBackgroundColor() : getBackgroundColor() * 0.9F);

		auto pos = Vector2f(this->_padding, (size.y - this->_textSize.y) / 2 + this->_offsetY);

		auto offset = this->_textSize.x + this->_charSize.x + this->_padding;
		if (offset > size.x) pos.x -= offset - size.x;

		if ((time(nullptr) % 2) == 0 && focused() && !this->_readOnly) {
			auto charPos = fnt->getStringSize(text.substr(0, Font::getByteCount(text, this->_charXPos)));
			stencil.drawBox({this->_padding + charPos.x + pos.x, 2}, {1, size.y - 4}, getColor());
		}

		if (!this->_hint.empty()) {
			stencil.drawText(fnt, this->_hint, {size.x - 5, pos.y}, Color::RGBAHex(0x000000A4), rawrbox::Alignment::Right);
		}

		if (text.empty() && !this->_readOnly) {
			stencil.drawText(fnt, this->_placeholder, {pos.x + 5, pos.y}, getColor() * 0.5F);
		} else {
			stencil.drawText(fnt, text, pos, getColor());
		}

		if (!this->_fillText.empty()) stencil.drawText(fnt, this->_fillText, {pos.x + this->_textSize.x, pos.y}, getColor() * 0.2F);

		// Border
		stencil.pushOutline({this->_borderSize});
		stencil.drawBox({}, size, getBorderColor());
		stencil.popOutline();
	}
	// ----------
} // namespace rawrbox
