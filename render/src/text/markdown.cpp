#include <rawrbox/render/text/markdown.hpp>

#include <array>

namespace rawrbox {
	void Markdown::pushTextSoFar(const std::string& text, size_t& textStart, size_t endPos) {
		MarkdownElement elm = _currentStyle;

		// skip empty entries
		if (text.empty() && elm.type == MarkdownType::text) {
			textStart = endPos + 1;
			return;
		}

		// append the element
		elm.text = {text.begin() + textStart, text.begin() + endPos};
		elm.size = elm.font->getStringSize(elm.text);
		_elements.push_back(elm);

		// advance our position
		textStart = endPos + 1;
	}

	std::vector<float> hexToBytes(const std::string& hex) {
		std::vector<float> bytes;

		for (size_t i = 0; i < hex.size(); i += 2) {
			bytes.push_back(static_cast<float>(
			    std::stol(hex.substr(i, 2), nullptr, 16)));
		}

		return bytes;
	}

	const rawrbox::Vector2f Markdown::getSize() const {
		rawrbox::Vector2f size = {};
		rawrbox::Vector2f curpos = {};

		for (auto& elm : _elements) {
			if (elm.type == MarkdownType::linebreak) {
				curpos.y += elm.size.y;
				curpos.x = 0;
				continue;
			}

			curpos.x += elm.size.x;
			if (curpos.x > size.x) size.x = curpos.x;
			if (curpos.y + elm.size.y > size.y) size.y = curpos.y + elm.size.y;
		}

		return size;
	}

	bool isHexString(const std::string& s) {
		return s.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
	}

	void Markdown::parseChunk(const std::string& text) {
		size_t textStart = 0;

		for (size_t i = 0; i < text.size(); i++) {
			// escape
			if (i > 0 && text[i] == '\\') continue;

			if (text[i] == '\n') {
				if (i > 0) pushTextSoFar(text, textStart, i);
				textStart = i + 1;

				// push linebreak element
				MarkdownElement elm;
				elm.type = MarkdownType::linebreak;
				elm.size.y = fontRegular->getLineHeight();
				_elements.push_back(elm);

				continue;
			}

			// bold and italic
			if (text[i] == '*') {
				// push text so far before changing the mode
				if (i > 0) pushTextSoFar(text, textStart, i);

				// figure out what mode
				bool bold = false;
				if (i + 1 < text.size() && text[i + 1] == '*') {
					bold = true;
					i++;
				}

				textStart = i + 1;

				// figure out what location to go to
				auto endPos = text.find(bold ? "**" : "*", i + 1);
				if (endPos == std::string::npos) continue;

				// set current style, do chunk and set style back
				auto old = _currentStyle.font;
				_currentStyle.font = bold ? fontBold : fontItalic;
				this->parseChunk({text.begin() + textStart, text.begin() + endPos});
				_currentStyle.font = old;

				// move our text locations
				i = endPos + 1;
				if (bold) i++;

				textStart = i;
				continue;
			}

			// corrupt
			if (text[i] == '~') {
				// push text so far before changing the mode
				if (i > 0) pushTextSoFar(text, textStart, i);
				textStart = i + 1;

				// figure out what location to go to
				auto endPos = text.find("~", i + 1);
				if (endPos == std::string::npos) continue;

				// set current style, do chunk and set style back
				auto old = _currentStyle.type;
				_currentStyle.type = MarkdownType::corrupt;
				this->parseChunk({text.begin() + textStart, text.begin() + endPos});
				_currentStyle.type = old;

				i = endPos;
				textStart = i + 1;
				continue;
			}

			// color [#324234][/]
			if (text[i] == '[') {
				// push text so far before changing the mode
				if (i > 0) pushTextSoFar(text, textStart, i);

				// figure out what location to go to
				auto endPos = text.find("]", i + 1);
				if (endPos == std::string::npos) continue; // []

				// get the color code string, if it's an end tag set it to default
				std::string colorStr = {text.begin() + i + 1, text.begin() + endPos};
				if (colorStr.empty()) continue;

				if (colorStr.front() != '#' && colorStr != "/") continue;

				if (colorStr == "/") {
					_currentStyle.color = _currentStyle.defaultColor;

					// advance in our string
					i = endPos;
					textStart = i + 1;

					continue;
				}

				colorStr = colorStr.substr(1);
				if (!isHexString(colorStr)) {
					continue;
				}

				// else basic validation of the string
				if (colorStr.size() % 2 == 1) continue;
				if (colorStr.size() > 8) continue;

				auto parts = hexToBytes(colorStr);
				if (parts.size() < 3) continue;

				// convert it to a color
				_currentStyle.color = {
				    parts[0] / 255.0F,
				    parts[1] / 255.0F,
				    parts[2] / 255.0F,
				    parts.size() > 3 ? (parts[3] / 255.0F) : 1.0F,
				};

				// advance in our string
				i = endPos;
				textStart = i + 1;
				continue;
			}
		}

		// push back last part of the text that didn't have special things
		this->pushTextSoFar(text, textStart, text.size());
	}

	void Markdown::clear() {
		this->_elements.clear();
	}

	void Markdown::parse(const std::string& text, rawrbox::Colorf defaultColor) {
		// init our default style
		this->_currentStyle.type = MarkdownType::text;
		this->_currentStyle.font = fontRegular;

		this->_currentStyle.defaultColor = defaultColor;
		this->_currentStyle.color = defaultColor;

		// begin the madness
		this->parseChunk(text);
	}

	void Markdown::render(rawrbox::Stencil* stencil, const rawrbox::Vector2f pos) {
		auto curpos = pos;

		// go trough every element
		for (auto& elm : _elements) {
			// if we're a linebreak then ajust position and continue
			if (elm.type == MarkdownType::linebreak) {
				curpos.y += elm.size.y;
				curpos.x = pos.x;
				continue;
			}

			// draw elements on screen
			if (elm.type == MarkdownType::corrupt) {
				auto& text = elm.text;

				std::array<wchar_t, 21> letters = {L'!', L'§', L'$', L'%', L'/', L'(', L')', L'=', L'?', L'_', L'<', L'>', L'^', L'°', L'*', L'#', L'-', L':', L';', L'~', L'Ø'};
				std::wstring corrupted;

				std::uniform_int_distribution<size_t> dist(0, letters.size() - 1);
				for (size_t i = 0; i < text.size(); i++) {
					corrupted.push_back(letters[dist(this->_prng)]);
				}

				// center text to match letter width changes
				auto corruptedUtf8 = elm.font->toUTF8(corrupted);
				auto midpos = curpos;
				midpos.x += elm.size.x / 2 - elm.font->getStringSize(corruptedUtf8).x / 2;

				// draw moddified text
				stencil->drawText(*elm.font, corruptedUtf8, midpos, elm.color, rawrbox::Alignment::Left);
			} else {
				// draw default text
				stencil->drawText(*elm.font, elm.text, curpos, elm.color, rawrbox::Alignment::Left);
			}

			curpos.x += elm.size.x;
		}
	}
} // namespace rawrbox
